#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <jansson.h>

#include "mcompat_config.h"
#include "mcompat_drv.h"
#include "mcompat_lib.h"

/* AGEING */
#define USE_VID_CALIBRATION

#define MINER_CONF_PATH         "/config/cgminer.conf"
#define MINER_CONF_BKP_PATH     "/config/.cgminer.conf"
#define AGEING_LOG_PATH         "/tmp/ageing.log"
#define AGEING_PROGRESS_PATH    "/tmp/ageing_progress.log"
#define AGEING_LOCK_PATH        "/tmp/ageing_lock"
#define AGEING_POOL             "stratum+tcp://pool.ckpool.org:3333"
#define AGEING_USER             "36JMs8rogpizjkkp6eeKMmm3SLbSNo5H9V"
#define AGEING_PASS             "x"
#define CGMINER_CMD             "/bin/cgminer -c /etc/cgminer.conf --api-listen --api-network --text-only --T1noauto"
#define KEY_PTN_PLL             "T1Pll%d"
#define KEY_PTN_VID             "T1Volt%d"

#define MAPI_HOST               "127.0.0.1"
#define MAPI_PORT               4028
#define MAPI_CMD_GETDEVS        "{\"command\" : \"devs\"}"
#define MAPI_CMD_GETSTATS       "{\"command\" : \"stats\"}"

#define TYPE_XHN
//#define TYPE_ZLE

#define CHAIN_NUM_MAX           (8)
#define CHAIN_NUM               (3)
#define PLL_LIST_MAX            (3)
#define VID_LIST_MAX            (6)
#define CHIP_VOLT_MAX           (500)           // unit: mV
#define CHIP_VOLT_MIN           (350)           // unit: mV
#define CONF_LEN_MAX            (4096)
#define TEST_START_TIME         (300)           // unit: seconds
#define TEST_PERIOD             (600)           // unit: seconds
#define TEST_TIME               (600)           // unit: seconds
#define VALIDATE_TIME           (300)           // unit: seconds
#define VALIDATE_CNT            (3)
#define TEST_PLL_CNT            (2)             // number of plls in test list
#define TEST_VID_CNT            (6)             // number of vids in test list
#define TEST_CNT_MAX            (TEST_VID_CNT + 1)// max test count
#define HS_THR_LEVEL_0          (15.70 * 1000 * 1000)   // unit: MHS
#define HS_THR_LEVEL_1          (15.30 * 1000 * 1000)   // unit: MHS
#define HS_THR_LEVEL_2          (14.00 * 1000 * 1000)   // unit: MHS
#define HS_THR_LEVEL_3          (12.00 * 1000 * 1000)   // unit: MHS
#define HS_THR_LEVEL_E1         (12.00 * 1000 * 1000)   // unit: MHS
#define HS_THR_LEVEL_E2         ( 3.00 * 1000 * 1000)   // unit: MHS
#define VOLT_THR_LEVEL_0        (412)           // unit: mV
#define VOLT_THR_LEVEL_1        (412)           // unit: mV
#define VOLT_THR_LEVEL_2        (415)           // unit: mV

#ifdef TYPE_XHN
#define VID_TARGET              (22)            // zle: 15, xhn: 22
#define VOL_TARGET              (410)           // zle: 401, xhn: 410
#else
#define VID_TARGET              (15)
#define VOL_TARGET              (401)
#endif

#define DM_LED_GREEN            (0x1)
#define DM_LED_RED              (0x2)
#define DM_LED_ON               (0)
#define DM_LED_OFF              (1)
#define DM_QUICK_FLASH_US       (250 * 1000)
#define DM_STARTUP_STEPS        (12)

typedef struct _c_chain_stat
{
    int is_valid;
    int chain_id;
    int pll;
    int vid;
    int local_acc;
    int accept;
    int reject;
    int hwerr;
    double hashrate;
} c_chain_stat;

typedef enum _DEV_LEVEL
{
    DEV_LEVEL_S = 0,
    DEV_LEVEL_1 = 1,
    DEV_LEVEL_2 = 2,
    DEV_LEVEL_3 = 3,
    DEV_LEVEL_4 = 4,
} DEV_LEVEL;

typedef enum _CHAIN_LEVEL
{
    CHAIN_LEVEL_A = 0,
    CHAIN_LEVEL_B = 1,
    CHAIN_LEVEL_C = 2,
} CHAIN_LEVEL;

typedef struct _c_led_blink
{
    uint32_t led;
    uint32_t blink_cnt;
    uint32_t wait;
    uint32_t loop;
} c_led_blink;

static const int g_pll_list[TEST_PLL_CNT] = {1332, 1296};
static int g_vid_list[CHAIN_NUM_MAX][TEST_VID_CNT];
static FILE *g_ageing_log = NULL;
static FILE *g_ageing_progress = NULL;
static int g_chain_max;
static int g_chain_cnt;
static bool g_chain_plug[CHAIN_NUM_MAX] = {0};

/* Leds Functions */
static void revert_led(int index);
static void set_red_led(int state);
static void set_green_led(int state);
static void set_chain_led(int chain_id, int state);
static void set_dev_led_blink(uint32_t led_mask, uint32_t blink_cnt, uint32_t wait, uint32_t loop);
static void set_chain_led_blink(uint32_t chain_id, uint32_t blink_cnt, uint32_t wait, uint32_t loop);
static void *thread_set_chain_led_blink(void *argv);
static void *thread_set_dev_led_blink(void *argv);

/* Ageing Functions */
static int  run_ageing(void);
static void stat_ageing_result(double *hashrate, int *chip_volt, int *dev_level, int *chain_level);
static void show_ageing_result(int dev_level, int *chain_level);
static bool api_get_chip_volt(int *chip_volt);
static int  api_get_chain_stat(c_chain_stat *stat);
static bool load_miner_conf(json_t **conf);
static bool save_miner_conf(json_t *conf, char *file_name);
static void write_ageing_log(char *text);
static void write_ageing_progress(char *text);
static int  call_miner_api(char *command, char *host, short int port, json_t **result);
static void chain_power_on(int chain_id, int type);
static int find_vid(int chain_id, int vid_start, int volt_target);


static void usage(const char *prog_name)
{
    printf("USAGE:\n");
    printf("      %s [--production].\n",prog_name);
    printf("      --production disable leds, restore user confs and restart cgminer\n");
}


static c_led_blink g_led_blink_cfg[CHAIN_NUM + 1];

static void *thread_set_dev_led_blink(void *argv)
{
    int i, j;
    c_led_blink *blink_cfg = (c_led_blink*) argv;
    uint32_t led_mask = blink_cfg->led;
    uint32_t blink_cnt = blink_cfg->blink_cnt;
    uint32_t wait = blink_cfg->wait;
    uint32_t loop = blink_cfg->loop;
/*
    char text[256] = {0};
    sprintf(text, "thread_set_dev_led_blink(): %d, %d, %d, %d\n", argv, led_mask, blink_cnt, wait, loop);
    write_ageing_log(text);
*/
    for(i = 0; i < loop; ++i)
    {
        // blink
        for(j = 0; j < blink_cnt; ++j)
        {
            if(led_mask & DM_LED_GREEN)
                set_green_led(DM_LED_ON);
            if(led_mask & DM_LED_RED)
                set_red_led(DM_LED_ON);
            usleep(500000);
            if(led_mask & DM_LED_GREEN)
                set_green_led(DM_LED_OFF);
            if(led_mask & DM_LED_RED)
                set_red_led(DM_LED_OFF);
            usleep(500000);
        }

        // wait
        sleep(wait);
    }
}

static void *thread_set_chain_led_blink(void *argv)
{
    int i, j;
    c_led_blink *blink_cfg = (c_led_blink*) argv;
    uint32_t chain_id = blink_cfg->led;
    uint32_t blink_cnt = blink_cfg->blink_cnt;
    uint32_t wait = blink_cfg->wait;
    uint32_t loop = blink_cfg->loop;
/*
    char text[256] = {0};
    sprintf(text, "thread_set_chain_led_blink(): %d, %d, %d, %d\n", chain_id, blink_cnt, wait, loop);
    write_ageing_log(text);
*/
    for(i = 0; i < loop; ++i)
    {
        // blink
        for(j = 0; j < blink_cnt; ++j)
        {
            set_chain_led(chain_id, DM_LED_ON);
            usleep(500000);
            set_chain_led(chain_id, DM_LED_OFF);
            usleep(500000);
        }

        // wait
        sleep(wait);
    }
}

static void set_dev_led_blink(uint32_t led_mask, uint32_t blink_cnt, uint32_t wait, uint32_t loop)
{
    pthread_t tid;

    g_led_blink_cfg[CHAIN_NUM].led = led_mask;
    g_led_blink_cfg[CHAIN_NUM].blink_cnt = blink_cnt;
    g_led_blink_cfg[CHAIN_NUM].wait = wait;
    g_led_blink_cfg[CHAIN_NUM].loop = loop;
/*
    char text[256] = {0};
    sprintf(text, "set_dev_led_blink(): %d, %d, %d, %d\n", led_mask, blink_cnt, wait, loop);
    write_ageing_log(text);
*/
    pthread_create(&tid, NULL, thread_set_dev_led_blink, &g_led_blink_cfg[CHAIN_NUM]);
}

static void set_chain_led_blink(uint32_t chain_id, uint32_t blink_cnt, uint32_t wait, uint32_t loop)
{
    pthread_t tid;

    g_led_blink_cfg[chain_id].led = chain_id;
    g_led_blink_cfg[chain_id].blink_cnt = blink_cnt;
    g_led_blink_cfg[chain_id].wait = wait;
    g_led_blink_cfg[chain_id].loop = loop;
/*
    char text[256] = {0};
    sprintf(text, "set_chain_led_blink(): %d, %d, %d, %d\n", chain_id, blink_cnt, wait, loop);
    write_ageing_log(text);
*/
    pthread_create(&tid, NULL, thread_set_chain_led_blink, &g_led_blink_cfg[chain_id]);
}

static void set_green_led(int state)
{
    mcompat_set_green_led(state);
}

static void set_red_led(int state)
{
    mcompat_set_red_led(state);
}

static void set_chain_led(int chain_id, int state)
{
    mcompat_set_led(chain_id, state);
}

int main(int argc, char *argv[])
{
    int rst = 0;
    
    //Check if the lock file exists
    if( access( AGEING_LOCK_PATH, F_OK ) != -1 ) {
        write_ageing_log("ERROR: ageing proccess is already running, terminating\n");
        return -1;
    }

    int retval = 0;
    int i, j, k, size, test_cnt;
    int chain_id, chip_id;
    int pll, vid;
    bool startup_flag;
    char text[256] = {0};
    char cmd0[1024] = {0};
    char cmd[2048] = {0};
    char key[32] = {0};
    char value[32] = {0};
    char *recv_buf = NULL;
    int volt;
    double hashrate;
    double hash_max[CHAIN_NUM_MAX] = {0};
    int chip_volt[CHAIN_NUM_MAX] = {0};
    int best_pll[CHAIN_NUM_MAX] = {0};
    int best_vid[CHAIN_NUM_MAX] = {0};
    int best_volt[CHAIN_NUM_MAX] = {0};
    int dev_level;
    int chain_level[CHAIN_NUM_MAX] = {0};
    int validate_cnt[CHAIN_NUM_MAX] = {0};
    bool validate_flag = false;
    c_chain_stat chain_stat[CHAIN_NUM_MAX] = {0};
    int additional_stops = 10;
    int global_step = DM_STARTUP_STEPS + 1; //1 for VID calibration

    json_t *p_json_cfg = NULL;
    
    /*
     * Check --production parameter,
     * if specified will not use mcompat_drv (no leds needed)
     * will restart cgminer when done and will create
     * a cgminer.conf backup
     */
    int production=0;
    if(argc >= 2 && strcmp (argv[1],"--production") == 0)
    {
        production=1;
    }

    // create lock file
    sprintf(text, "touch %s", AGEING_LOCK_PATH);
    system(text);

    // record ageing timestamp
    sprintf(text, "date > %s", AGEING_LOG_PATH);
    system(text);
    
    // clean progress file
    sprintf(text, "echo \"\" > %s", AGEING_PROGRESS_PATH);
    system(text);
    
    if (production==1) {
        sprintf(text, "Steps=%d\n", (TEST_CNT_MAX*additional_stops)+DM_STARTUP_STEPS+1);//Make it ends in 99%
        write_ageing_progress(text);
    }

    // detect chain number
    sys_platform_init(PLATFORM_ZYNQ_HUB_G19, MCOMPAT_LIB_MINER_TYPE_T1, 
        MCOMPAT_CONFIG_MAX_CHAIN_NUM, MCOMPAT_CONFIG_MAX_CHIP_NUM);
    sys_platform_debug_init(MCOMPAT_LOG_ERR);
    g_chain_cnt = g_chain_max = 0;
    for(k = 0; k < CHAIN_NUM_MAX; ++k)
    {
        if(mcompat_get_plug(k) == 0)
        {
            g_chain_plug[k] = true;
            g_chain_max = k;
            g_chain_cnt++;
        }
        else
        {
            g_chain_plug[k] = false;
        }
    }
    sys_platform_exit();

#ifndef USE_CGMINER_AGEING 
    // find best startup mode
    write_ageing_log("detect best startup mode\n");
    system("systemctl restart cgminer.service");

    for(i = 0; i < DM_STARTUP_STEPS; ++i)
    {
        if (production==1) {
            sprintf(text, "Step=%d\n", i+1);
            write_ageing_progress(text);
        }
        
        sleep(60);     // wait 1 min
        size = api_get_chain_stat(chain_stat);
        startup_flag = false;
        for(j = 0; j < size; ++j)
        {
            if(chain_stat[j].local_acc > 0)
            {
                startup_flag = true;
                break;
            }
        }
        if(startup_flag)
        {
            write_ageing_log("    finished\n");
            break;
        }
    }

    sprintf(text, "Step=%d\n", global_step);
    write_ageing_progress(text);

    // stop cgminer
    write_ageing_log("    stop cgminer\n");
    system("systemctl stop cgminer.service");
    sleep(5);

#ifdef USE_VID_CALIBRATION
    sys_platform_init(PLATFORM_ZYNQ_HUB_G19, MCOMPAT_LIB_MINER_TYPE_T1, 
        MCOMPAT_CONFIG_MAX_CHAIN_NUM, MCOMPAT_CONFIG_MAX_CHIP_NUM);
    sys_platform_debug_init(MCOMPAT_LOG_ERR);

    // vid calibration
    for(k = 0; k <= g_chain_max; ++k)
    {
        if(g_chain_plug[k])
        {
//            g_vid_list[k][0] = find_vid(k, VID_TARGET, VOL_TARGET);
            g_vid_list[k][0] = VOL_TARGET;
            g_vid_list[k][1] = g_vid_list[k][0] + 6;
            g_vid_list[k][2] = g_vid_list[k][0] + 3;
            g_vid_list[k][3] = g_vid_list[k][0] - 2;
            g_vid_list[k][4] = g_vid_list[k][0] - 4;
            g_vid_list[k][5] = g_vid_list[k][0] - 6;
            sprintf(text, "    [chain%d] vidlist: %d, %d, %d, %d, %d, %d\n", k, g_vid_list[k][0], 
                g_vid_list[k][1], g_vid_list[k][2], g_vid_list[k][3], g_vid_list[k][4], g_vid_list[k][5]);
            write_ageing_log(text);
        }
    }

    sys_platform_exit();
#else
    for(k = 0; k <= g_chain_max; ++k)
    {
//        g_vid_list[k][0] = VID_TARGET;
        g_vid_list[k][0] = VOL_TARGET;
        g_vid_list[k][1] = g_vid_list[k][0] + 6;
        g_vid_list[k][2] = g_vid_list[k][0] + 3;
        g_vid_list[k][3] = g_vid_list[k][0] - 2;
        g_vid_list[k][4] = g_vid_list[k][0] - 4;
        g_vid_list[k][4] = g_vid_list[k][0] - 6;
    }
#endif

    // try every combination of pll and vid
    test_cnt = 0;
    for(i = 0; i < TEST_PLL_CNT; ++i)
    {
        // try no more than 4 combinations
        if(test_cnt >= TEST_CNT_MAX)
            break;

        // set pll
        strcpy(cmd0, CGMINER_CMD);
        for(k = 0; k <= g_chain_max; ++k)
        {
            sprintf(key, " --" KEY_PTN_PLL " %d", k + 1, g_pll_list[i]);
            strcat(cmd0, key);
        }

        for(j = 0; j < TEST_VID_CNT; ++j)
        {
            // try no more than 4 combinations
            if(test_cnt >= TEST_CNT_MAX)
                break;

            test_cnt++;
            sprintf(text, "<Round%d> pll=%d", test_cnt, g_pll_list[i]);
            write_ageing_log(text);

            // set vid
            strcpy(cmd, cmd0);
            for(k = 0; k <= g_chain_max; ++k)
            {
                sprintf(key, " --" KEY_PTN_VID " %d", k + 1, g_vid_list[k][j]);
                strcat(cmd, key);
                sprintf(text, ", vid%d=%d", k, g_vid_list[k][j]);
                write_ageing_log(text);
            }
            write_ageing_log("\n");

            // start miner
            strcat(cmd, " &");      // run in background
            write_ageing_log("    start miner\n");
//            sprintf(text, "    %s\n", cmd);
//            write_ageing_log(text);
            system(cmd);

            sprintf(text, "    wait for %ds\n", TEST_PERIOD);
            write_ageing_log(text);
            if (production==0) {
                sleep(TEST_PERIOD);
            } else {
                int period_stops=TEST_PERIOD/additional_stops;
                int h;
                for (h=0;h<additional_stops;h++) {
                    sprintf(text, "Step=%d\n", global_step);
                    write_ageing_progress(text);
                    sleep(period_stops);
                    global_step++;
                }
            }

            // get voltage of each chip
            if(!api_get_chip_volt(chip_volt))
            {
                write_ageing_log("    failed to read voltage, ingore current pll and vid\n");
                goto AGEING_ROUND_END;
            }

            // get chain stat.
            if(api_get_chain_stat(chain_stat) == 0)
            {
                write_ageing_log("    failed to read hashrate, ingore current pll and vid\n");
                goto AGEING_ROUND_END;
            }

            // result collect.
            for(k = 0; k <= g_chain_max; ++k)
            {
                if(g_chain_plug[k] && chip_volt[k] != 0 && chain_stat[k].hashrate > hash_max[k])
                {
                    hash_max[k] = chain_stat[k].hashrate;
                    best_pll[k] = g_pll_list[i];
                    best_vid[k] = g_vid_list[k][j];
                    best_volt[k] = chip_volt[k];
                }

                sprintf(text, "    [chain%d]: ACC1=%d, ACC2=%d, REJ=%d, HW=%d, HS=%.2f, MAX_HASH=%.2f, BEST_PLL=%d, BEST_VID=%d, CHIP_VOLT=%d\n", 
                    k, chain_stat[k].accept, chain_stat[k].local_acc, chain_stat[k].reject, chain_stat[k].hwerr, chain_stat[k].hashrate, 
                    hash_max[k], best_pll[k], best_vid[k], best_volt[k]);
                write_ageing_log(text);
            }

AGEING_ROUND_END:
            // stop miner
            write_ageing_log("    stop miner\n");
            system("killall cgminer");
            sleep(5);
        }
    }

    // validate new best pll and vid
    write_ageing_log("Try new best pll and vid:\n");
    strcpy(cmd, CGMINER_CMD);
    for(k = 0; k <= g_chain_max; ++k)
    {
        sprintf(key, " --" KEY_PTN_PLL " %d", k + 1, best_pll[k]);
        strcat(cmd, key);
        sprintf(key, " --" KEY_PTN_VID " %d", k + 1, best_volt[k]);
        strcat(cmd, key);
    }
    strcat(cmd, " &");      // run in background
    validate_flag = true;
    for(i = 0; i < VALIDATE_CNT && validate_flag; ++i)
    {
        sprintf(text, "    <Round%d>\n", i + 1);
        write_ageing_log(text);
        // run miner
        system(cmd);
        sleep(VALIDATE_TIME);

        // get acc stat.
        if(api_get_chain_stat(chain_stat) == CHAIN_NUM)
        {
            for(k = 0; k < CHAIN_NUM; ++k)
            {
                if(chain_stat[k].local_acc == 0)
                { // chain k failed
                    validate_flag = false;
                    hash_max[k] = 0;    // set best hash to 0 which indicates level 4
                    sprintf(text, "        chain%d: failed\n", k);
                    write_ageing_log(text);
                }
                else
                { // chain k succeed
                    sprintf(text, "        chain%d: OK\n", k);
                    write_ageing_log(text);
                }
            }
        }
        else
        { // all chain failed
            for(k = 0; k < CHAIN_NUM; ++k)
            {
                if(chain_stat[k].is_valid)
                {
                    sprintf(text, "        chain%d: OK\n", k);
                    write_ageing_log(text);
                }
                else
                {
                    hash_max[k] = 0;    // set best hash to 0 which indicates level 4
                    sprintf(text, "        chain%d: failed\n", k);
                    write_ageing_log(text);
                }
            }
            validate_flag = false;
        }

        // stop miner
        system("killall cgminer");
        sleep(5);
    }

    // determine device level and chain level by hashrate and chip voltage
    write_ageing_log("<Result>\n");
    stat_ageing_result(hash_max, best_volt, &dev_level, chain_level);

    if(!load_miner_conf(&p_json_cfg))
    {
        retval = -1;
        goto AGEING_EXIT;
    }

    // write best pll and vid to miner conf
    for(k = 0; k <= g_chain_max; ++k)
    {
        if(best_pll[k] > 0)
        {
            sprintf(key, KEY_PTN_PLL, k + 1);
            sprintf(value, "%d", best_pll[k]);
            json_object_set_new(p_json_cfg, key, json_string(value));
            sprintf(text, "    [chain%d]: %s=%s\n", k, key, value);
            write_ageing_log(text);
        }

        if(best_volt[k] > 0)
        {
            sprintf(key, KEY_PTN_VID, k + 1);
            sprintf(value, "%d", best_volt[k]);
            json_object_set_new(p_json_cfg, key, json_string(value));
            sprintf(text, "    [chain%d]: %s=%s\n", k, key, value);
            write_ageing_log(text);
        }
    }
    write_ageing_log("saving miner config\n");
    save_miner_conf(p_json_cfg, MINER_CONF_PATH);
    write_ageing_log("saving backup miner config\n");
    save_miner_conf(p_json_cfg, MINER_CONF_BKP_PATH);

    if(p_json_cfg)
        json_decref(p_json_cfg);

    //Start CgMiner Again
    if (production == 1)
    {
        write_ageing_log("start miner\n");
        system("systemctl start cgminer.service");
    }
#else
    // start cgminer
    write_ageing_log("USE_CGMINER_AGEING\nstart miner\n");
    system("systemctl restart cgminer.service");

    // ignore first stage result
    sprintf(text, "ignore first stage result:\n    wait for %ds\n", TEST_START_TIME);
    write_ageing_log(text);
    sleep(TEST_START_TIME);

    // set init level
    dev_level = DEV_LEVEL_4;
    for(i = 0; i < CHAIN_NUM; ++i)
        chain_level[i] = DEV_LEVEL_C;

    for(i = 0; i < TEST_TIME / TEST_PERIOD; ++i)
    {
        sprintf(text, "check hashrate(%d):\n    wait for %ds\n", i, TEST_PERIOD);
        write_ageing_log(text);
        sleep(TEST_PERIOD);

        // get voltage of each chip
        if(!api_get_chip_volt(chip_volt))
        {
            write_ageing_log("    failed to read chip voltage. restart cgminer\n");
            system("systemctl restart cgminer.service");
            continue;
        }

        // get chain stat.
        if(api_get_chain_stat(chain_stat) == 0)
        {
            write_ageing_log("    failed to read hashrate. restart cgminer\n");
            system("systemctl restart cgminer.service");
            continue;
        }

        // determine device level and chain level by hashrate and chip voltage
        for(k = 0; k < CHAIN_NUM_MAX; ++k)
            hash_max[k] = chain_stat[k].hashrate;
        stat_ageing_result(hash_max, chip_volt, &dev_level, chain_level);

        if(dev_level == DEV_LEVEL_S)
            break;
    }

    // stop cgminer to avoid to change led state in two different process
    write_ageing_log("stop miner\n");
    system("systemctl stop cgminer.service");
    sleep(5);
#endif  // #ifdef USE_CGMINER_AGEING

    if (production==0)
        show_ageing_result(dev_level, chain_level);

AGEING_EXIT:

    if(retval == -1 && production == 0)
    {
        set_green_led(DM_LED_OFF);
        set_red_led(DM_LED_OFF);
        for(i = 0; i < CHAIN_NUM; ++i)
            set_chain_led(i, DM_LED_OFF);
    }

    // remove lock file
    write_ageing_log("removing lock file\n");
    sprintf(text, "rm -f %s", AGEING_LOCK_PATH);
    system(text);

    write_ageing_log("ageing done\n");
    
    if(g_ageing_log)
    {
        fprintf(g_ageing_log, "\n\n");
        fclose(g_ageing_log);
    }
    
    // Allow blinking threads keep running
    if (production==0) {
        while(true)
           sleep(1);
    }

    return retval;
}

static void write_ageing_log(char *text)
{
    if(NULL == g_ageing_log)
    {
        g_ageing_log = fopen(AGEING_LOG_PATH, "a");
        if(NULL == g_ageing_log)
        {
            fprintf(stderr, "ERROR: failed to open ageing log file %s\n", AGEING_LOG_PATH);
            return;
        }
    }

    fprintf(stdout, text);
    fprintf(g_ageing_log, text);
    fflush(g_ageing_log);
}
static void write_ageing_progress(char *text)
{
    if(NULL == g_ageing_progress)
    {
        g_ageing_progress = fopen(AGEING_PROGRESS_PATH, "a");
        if(NULL == g_ageing_progress)
        {
            fprintf(stderr, "ERROR: failed to open ageing progress file %s\n", AGEING_PROGRESS_PATH);
            return;
        }
    }

    fprintf(g_ageing_progress, text);
    fflush(g_ageing_progress);
}

static bool load_miner_conf(json_t **conf)
{
    int size;
    char buffer[CONF_LEN_MAX] = {0};
    json_error_t error;

    FILE *p_file = fopen(MINER_CONF_PATH, "r");
    if(NULL == p_file)
    {
        write_ageing_log("ERROR: failed to open cgminer conf\n");
        return false;
    }

    size = fread(buffer, 1, CONF_LEN_MAX, p_file);
    fclose(p_file);
    if(size <= 0)
    {
        write_ageing_log("ERROR: failed to read cgminer conf\n");
        return false;
    }

    *conf = json_loads(buffer, 0, &error);
    if (NULL == *conf)
    {
        write_ageing_log("ERROR: failed to parse cgminer conf\n");
        return false;
    }

    return true;
}

static bool save_miner_conf(json_t *conf, char *file_name)
{
    int size;
    char buffer[CONF_LEN_MAX] = {0};
    FILE *p_file = NULL;

    size = json_dumpb(conf, buffer, CONF_LEN_MAX, JSON_INDENT(4));
    if(size <= 0)
    {
        write_ageing_log("ERROR: failed to parse cgminer conf\n");
        return false;
    }

    p_file = fopen(file_name, "w");
    if(NULL == p_file)
    {
        write_ageing_log("ERROR: failed to open cgminer conf\n");
        return false;
    }

    if(fwrite(buffer, 1, size, p_file) == size)
    {
        fclose(p_file);
        return true;
    }
    else
    {
        write_ageing_log("ERROR: failed to write cgminer conf\n");
        fclose(p_file);
        return false;
    }
}

static int call_miner_api(char *command, char *host, short int port, json_t **result)
{
    
    struct hostent *ip;
    struct sockaddr_in serv;
    int sock;
    int ret = 0;
    int n = 0;
    char *buf = NULL;
    size_t len = 16383;
    size_t p = 0;
    json_error_t error;

    ip = gethostbyname(host);
	if (!ip) {
		write_ageing_log("ERROR: couldn't get hostname\n");
		return 1;
	}

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        write_ageing_log("ERROR: socket initialisation failed\n");
        return -1;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr = *((struct in_addr *)ip->h_addr);
    serv.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0)
    {
        write_ageing_log("ERROR: socket connect failed\n");
        return -1;
    }

    n = send(sock, command, strlen(command), 0);
    if (n < 0)
    {
        write_ageing_log("ERROR: send failed\n");
        ret = -1;
    }
    else
    {
        buf = malloc(len+1);
        if (!buf)
        {
            write_ageing_log("ERROR: OOM\n");
            return -1;
        }

        while(1)
        {
            if ((len - p) < 1)
            {
                len += 16383;
                buf = realloc(buf, len+1);
                if (!buf)
                {
                    write_ageing_log("ERROR: OOM\n");
                    return -1;
                }
            }

            n = recv(sock, &buf[p], len - p , 0);
            if(n < 0)
            {
                write_ageing_log("ERROR: recv failed\n");
                ret = -1;
                break;
            }

            if(0 == n)
            {
                break;
            }

            p += n;
        }
        buf[p] = '\0';

        *result = json_loads(buf, 0, &error);
        free(buf);
        if (NULL == *result)
        {
            write_ageing_log("ERROR: failed to parse api message\n");
            ret = -1;
        }
    }

    close(sock);

    return ret;
}

static bool api_get_chip_volt(int *chip_volt)
{
    int i;
    int size;
    int chain_id, chip_id, chip_num;
    char key[64] = {0};
    char text[64] = {0};
    json_t *p_json_rst = NULL;
    json_t *p_json_obj = NULL;
    json_t *p_json_arr = NULL;
    json_t *p_json_val = NULL;

    // get voltage of each chip
    if(call_miner_api(MAPI_CMD_GETSTATS, MAPI_HOST, MAPI_PORT, &p_json_rst) < 0)
    {
        return false;
    }

    // chip voltage stat.
    p_json_arr = json_object_get(p_json_rst, "STATS");
    size = json_array_size(p_json_arr);
    for(i = 0; i < size; ++i)
    {
        p_json_obj = json_array_get(p_json_arr, i);
        p_json_val = json_object_get(p_json_obj, "Chain ID");
        if(!p_json_val)
            continue;

        chain_id = json_integer_value(p_json_val);
        chip_volt[chain_id] = 0;

        p_json_val = json_object_get(p_json_obj, "Num active chips");
        if(p_json_val && (chip_num = json_integer_value(p_json_val)) > 0)
        { // active chain
            for(chip_id = 0; chip_id < chip_num; ++chip_id)
            {
                sprintf(key, "%02d nVol", chip_id);
                chip_volt[chain_id] += json_integer_value(json_object_get(p_json_obj, key));
            }
            chip_volt[chain_id] /= chip_num;
        }
        else
        { // chain disabled or active chip num is zero
            sprintf(text, "ERROR: failed to get chip num for chain %d\n", i);
            write_ageing_log(text);
        }
    }

    if(p_json_rst)
        json_decref(p_json_rst);
    
    return true;
}

static int api_get_chain_stat(c_chain_stat *stat)
{
    int i;
    int size;
    int chain_id;
    json_t *p_json_rst = NULL;
    json_t *p_json_obj = NULL;
    json_t *p_json_arr = NULL;
    json_t *p_json_val = NULL;

    for(i = 0; i < CHAIN_NUM; ++i)
        stat[i].is_valid = 0;

    // get ageing result
    if(call_miner_api(MAPI_CMD_GETDEVS, MAPI_HOST, MAPI_PORT, &p_json_rst) < 0)
    {
        return 0;
    }

    // result stat.
    p_json_arr = json_object_get(p_json_rst, "DEVS");
    size = json_array_size(p_json_arr);
    for(i = 0; i < size; ++i)
    {
        p_json_obj = json_array_get(p_json_arr, i);
        chain_id = json_integer_value(json_object_get(p_json_obj, "ASC"));
        stat[chain_id].chain_id  = chain_id;
        stat[chain_id].is_valid  = 1;
        stat[chain_id].local_acc = json_integer_value(json_object_get(p_json_obj, "Diff1 Work"));     // local accepted
        stat[chain_id].accept    = json_integer_value(json_object_get(p_json_obj, "Accepted"));       // pool accepted
        stat[chain_id].reject    = json_integer_value(json_object_get(p_json_obj, "Rejected"));       // pool rejected
        stat[chain_id].hwerr     = json_integer_value(json_object_get(p_json_obj, "Hardware Errors"));// hardware error
        stat[chain_id].hashrate  = json_real_value(json_object_get(p_json_obj, "MHS av"));            // average hashrate
    }

    if(p_json_rst)
        json_decref(p_json_rst);
    
    return size;
}

static void stat_ageing_result(double *hashrate, int *chip_volt, int *dev_level, int *chain_level)
{
    int k, cnt;
    int volt_avg = 0;
    double hash_total = 0;
    char text[256] = {0};

    // determine chain level (A/B/C)
    cnt = 0;
    for(k = 0; k <= g_chain_max; ++k)
    {
        if(!g_chain_plug[k])
            continue;

        if(chip_volt[k] < CHIP_VOLT_MIN)
        {
            chain_level[k] = CHAIN_LEVEL_C;     // level C
            continue;
        }

        hash_total += hashrate[k];
        volt_avg += chip_volt[k];
        cnt++;

        if(hashrate[k] * CHAIN_NUM > HS_THR_LEVEL_E1)
        {
            chain_level[k] = CHAIN_LEVEL_A;     // level A: good chain
        }
        else if(hashrate[k] * CHAIN_NUM > HS_THR_LEVEL_E2)
        {
            chain_level[k] = CHAIN_LEVEL_B;     // level B: low hashrate
        }
        else
        {
            chain_level[k] = CHAIN_LEVEL_C;     // level C: broken chain
        }

        sprintf(text, "    [chain%d]: LEVEL%d - HASH_RATE=%.2f, CHIP_VOLT=%d\n", 
            k, chain_level[k], hashrate[k], chip_volt[k]);
        write_ageing_log(text);
    }
    volt_avg = (cnt > 0) ? (volt_avg /= cnt) : 0;

    // determine device level (S/1/2/3/4)
    if(volt_avg < CHIP_VOLT_MIN)
    {
        *dev_level = DEV_LEVEL_4;    // level 4
    }
    else if(hash_total >= HS_THR_LEVEL_0 && volt_avg <= VOLT_THR_LEVEL_0)
    {
        *dev_level = DEV_LEVEL_S;    // level S
    }
    else if(hash_total >= HS_THR_LEVEL_1 && volt_avg <= VOLT_THR_LEVEL_1)
    {
        *dev_level = DEV_LEVEL_1;    // level 1
    }
    else if(hash_total >= HS_THR_LEVEL_2 && volt_avg <= VOLT_THR_LEVEL_2)
    {
        *dev_level = DEV_LEVEL_2;    // level 2
    }
    else if(hash_total >= HS_THR_LEVEL_3)
    {
        *dev_level = DEV_LEVEL_3;    // level 3
    }
    else
    {
        *dev_level = DEV_LEVEL_4;    // level 4
    }
    sprintf(text, "    [device]: LEVEL%d - HASH_RATE=%.2f, CHIP_VOLT=%d\n", 
        *dev_level, hash_total, volt_avg);
    write_ageing_log(text);
}

static void show_ageing_result(int dev_level, int *chain_level)
{
    int i;
    char text[128] = {0};

    write_ageing_log("Showing ageing results\n");

    sys_platform_init(PLATFORM_ZYNQ_HUB_G19, MCOMPAT_LIB_MINER_TYPE_T1, 
        MCOMPAT_CONFIG_MAX_CHAIN_NUM, MCOMPAT_CONFIG_MAX_CHIP_NUM);
    sys_platform_debug_init(MCOMPAT_LOG_ERR);

    // show chain level (A/B/C) by led blinking
    for(i = 0; i < CHAIN_NUM; ++i)
    {
        if(!g_chain_plug[i])
            continue;

        switch(chain_level[i])
        {
        case CHAIN_LEVEL_A:
            sprintf(text, "    [chain%d]: LEVEL A\n", i);
            write_ageing_log(text);
            set_chain_led(i, DM_LED_OFF);
            break;
        case CHAIN_LEVEL_B:
            sprintf(text, "    [chain%d]: LEVEL B\n", i);
            write_ageing_log(text);
            set_chain_led_blink(i, 3, 2, -1);   // blink 3 times and then sleep 2s
            break;
        case CHAIN_LEVEL_C:
            sprintf(text, "    [chain%d]: LEVEL C\n", i);
            write_ageing_log(text);
            set_chain_led(i, DM_LED_ON);
            break;
        }
    }

    // show device level (S/1/2/3/4) by led blinking
    switch(dev_level)
    {
    case DEV_LEVEL_S:
        write_ageing_log("Device Level S\n");
        set_green_led(DM_LED_OFF);
        set_dev_led_blink(DM_LED_RED, -1, 0, -1);
        break;
    case DEV_LEVEL_1:
        write_ageing_log("Device Level 1\n");
        set_green_led(DM_LED_ON);
        set_red_led(DM_LED_ON);
        break;
    case DEV_LEVEL_2:
        write_ageing_log("Device Level 2\n");
        set_green_led(DM_LED_ON);
        set_dev_led_blink(DM_LED_RED, -1, 0, -1);
        break;
    case DEV_LEVEL_3:
        write_ageing_log("Device Level 3\n");
        set_red_led(DM_LED_ON);
        set_dev_led_blink(DM_LED_GREEN, -1, 0, -1);
        break;
    case DEV_LEVEL_4:
        write_ageing_log("Device Level 4\n");
        set_dev_led_blink(DM_LED_GREEN | DM_LED_RED, -1, 0, -1);
        break;
    }
}

/****************************************************************************************
 * volt calibration
 ****************************************************************************************/
typedef enum _POWERON_TYPES
{
    PWON_TYPE_1   = 0,
    PWON_TYPE_2   = 1,
    PWON_TYPE_MAX = 2
} POWERON_TYPES;

const uint8_t default_reg[142][12] =
{
	{0x02, 0x50, 0x40, 0xc2, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //120 MHz
	{0x02, 0x53, 0x40, 0xc2, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //125 MHz
	{0x02, 0x56, 0x40, 0xc2, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //129 MHz
	{0x02, 0x5d, 0x40, 0xc2, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //140 MHz
	{0x02, 0x35, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //159 MHz
	{0x02, 0x39, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //171 MHz
	{0x02, 0x3c, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //180 MHz
	{0x02, 0x3f, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //189 MHz
	{0x02, 0x43, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //201 MHz
	{0x02, 0x46, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //210 MHz
	{0x02, 0x49, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //219 MHz
	{0x02, 0x4d, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //231 MHz
	{0x02, 0x50, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //240 MHz
	{0x02, 0x53, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //249 MHz
	{0x02, 0x57, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //261 MHz
	{0x02, 0x5a, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //270 MHz
	{0x02, 0x5d, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //279 MHz
	{0x02, 0x61, 0x40, 0x82, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //291 MHz
	{0x02, 0x32, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //300 MHz
	{0x02, 0x34, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //312 MHz
	{0x02, 0x35, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //318 MHz
	{0x02, 0x37, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //330 MHz
	{0x02, 0x39, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //342 MHz
	{0x02, 0x3a, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //348 MHz
	{0x02, 0x3c, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //360 MHz
	{0x02, 0x3e, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //372 MHz
	{0x02, 0x3f, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //378 MHz
	{0x02, 0x41, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //390 MHz
	{0x02, 0x43, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //402 MHz
	{0x02, 0x44, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //408 MHz
	{0x02, 0x46, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //420 MHz
	{0x02, 0x48, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //432 MHz
	{0x02, 0x49, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //438 MHz
	{0x02, 0x4b, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //450 MHz
	{0x02, 0x4d, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //462 MHz
	{0x02, 0x4e, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //468 MHz
	{0x02, 0x50, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //480 MHz
	{0x02, 0x52, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //492 MHz
	{0x02, 0x53, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //498 MHz
	{0x02, 0x55, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //510 MHz
	{0x02, 0x57, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //522 MHz
	{0x02, 0x58, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //528 MHz
	{0x02, 0x5a, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //540 MHz
	{0x02, 0x5c, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //552 MHz
	{0x02, 0x5d, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //558 MHz
	{0x02, 0x5f, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //570 MHz
	{0x02, 0x61, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //582 MHz
	{0x02, 0x62, 0x40, 0x42, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //588 MHz
	{0x02, 0x32, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x24, 0xff, 0xff},  //600 MHz
	{0x02, 0x33, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //612 MHz
	{0x02, 0x34, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //624 MHz
	{0x04, 0x69, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //630 MHz
	{0x02, 0x35, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //636 MHz
	{0x02, 0x36, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //648 MHz
	{0x02, 0x37, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //660 MHz
	{0x02, 0x38, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //672 MHz
	{0x02, 0x39, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //684 MHz
	{0x04, 0x73, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //690 MHz
	{0x02, 0x3a, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //696 MHz
	{0x02, 0x3b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //708 MHz
	{0x02, 0x3c, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //720 MHz
	{0x02, 0x3d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //732 MHz
	{0x02, 0x3e, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //744 MHz
	{0x04, 0x7d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //750 MHz
	{0x02, 0x3f, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //756 MHz
	{0x02, 0x40, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //768 MHz
	{0x02, 0x41, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //780 MHz
	{0x02, 0x42, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //792 MHz
	{0x02, 0x43, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //804 MHz
	{0x04, 0x87, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //810 MHz
	{0x02, 0x44, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //816 MHz
	{0x02, 0x45, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //828 MHz
	{0x02, 0x46, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //840 MHz
	{0x02, 0x47, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //852 MHz
	{0x02, 0x48, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //864 MHz
	{0x04, 0x91, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //870 MHz
	{0x02, 0x49, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //876 MHz
	{0x02, 0x4a, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //888 MHz
	{0x02, 0x4b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //900 MHz
	{0x02, 0x4c, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //912 MHz
	{0x02, 0x4d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //924 MHz
	{0x04, 0x9b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //930 MHz
	{0x02, 0x4e, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //936 MHz
	{0x02, 0x4f, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //948 MHz
	{0x02, 0x50, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //960 MHz
	{0x02, 0x51, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //972 MHz
	{0x02, 0x52, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //984 MHz
	{0x04, 0xa5, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //990 MHz
	{0x02, 0x53, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //996 MHz
	{0x02, 0x54, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1008 MHz
	{0x02, 0x55, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1020 MHz
	{0x02, 0x56, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1032 MHz
	{0x02, 0x57, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1044 MHz
	{0x04, 0xaf, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1050 MHz
	{0x02, 0x58, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1056 MHz
	{0x02, 0x59, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1068 MHz
	{0x02, 0x5a, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1080 MHz
	{0x02, 0x5b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1092 MHz
	{0x02, 0x5c, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1104 MHz
	{0x04, 0xb9, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1110 MHz
	{0x02, 0x5d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1116 MHz
	{0x02, 0x5e, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1128 MHz
	{0x02, 0x5f, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1140 MHz
	{0x02, 0x60, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1152 MHz
	{0x02, 0x61, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1164 MHz
	{0x04, 0xc3, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1170 MHz
	{0x02, 0x62, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1176 MHz
	{0x02, 0x63, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1188 MHz
	{0x02, 0x64, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1200 MHz
	{0x02, 0x65, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1212 MHz
	{0x02, 0x66, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1224 MHz
	{0x02, 0x67, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1236 MHz
	{0x02, 0x68, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1248 MHz
	{0x02, 0x69, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1260 MHz
	{0x02, 0x6a, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1272 MHz
	{0x02, 0x6b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1284 MHz
	{0x02, 0x6c, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1296 MHz
	{0x02, 0x6d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1308 MHz
	{0x02, 0x6e, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1320 MHz
	{0x02, 0x6f, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1332 MHz
	{0x02, 0x70, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1344 MHz
	{0x02, 0x71, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1356 MHz
	{0x02, 0x72, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1368 MHz
	{0x02, 0x73, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1380 MHz
	{0x02, 0x74, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1392 MHz
	{0x02, 0x75, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1404 MHz
	{0x02, 0x76, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1416 MHz
	{0x02, 0x77, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1428 MHz
	{0x02, 0x78, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1440 MHz
	{0x02, 0x79, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1452 MHz
	{0x02, 0x7a, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1464 MHz
	{0x02, 0x7b, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1476 MHz
	{0x02, 0x7c, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1488 MHz
	{0x02, 0x7d, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1500 MHz
	{0x02, 0x7e, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1512 MHz
	{0x02, 0x7f, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1524 MHz
	{0x02, 0x80, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1536 MHz
	{0x02, 0x81, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1548 MHz
	{0x02, 0x82, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1560 MHz
	{0x02, 0x83, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1572 MHz
	{0x02, 0x84, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff},  //1584 MHz
	{0x02, 0x85, 0x40, 0x02, 0x00, 0x00, 0x80, 0xa0, 0x00, 0x20, 0xff, 0xff}   //1596 MHz
};

#define T1_PLL(prediv,fbdiv,postdiv) ((prediv<<(89-64))|fbdiv<<(80-64)|0b010<<(77-64)|postdiv<<(70-64))
struct PLL_Clock {
	unsigned int num;   // divider 1000
	unsigned int speedMHz;      // unit MHz
	unsigned int pll_reg;
};

const struct PLL_Clock PLL_Clk_12Mhz[142]={
	{0,   120,  T1_PLL(1, 80, 3)}, //default
	{1,   125,  T1_PLL(1, 83, 3)}, //
	{2,   129,  T1_PLL(1, 86, 3)},
	{3,   140,  T1_PLL(1, 93, 3)},
	{4,   159,  T1_PLL(1, 53, 2)},
	{5,   171,  T1_PLL(1, 57, 2)},
	{6,   180,  T1_PLL(1, 60, 2)},
	{7,   189,  T1_PLL(1, 63, 2)},
	{8,   201,  T1_PLL(1, 67, 2)},
	{9,   210,  T1_PLL(1, 70, 2)},
	{10,  219,  T1_PLL(1, 73, 2)},
	{11,  231,  T1_PLL(1, 77, 2)},
	{12,  240,  T1_PLL(1, 80, 2)},
	{13,  249,  T1_PLL(1, 83, 2)},
	{14,  261,  T1_PLL(1, 87, 2)},
	{15,  270,  T1_PLL(1, 90, 2)},
	{16,  279,  T1_PLL(1, 93, 2)},
	{17,  291,  T1_PLL(1, 97, 2)},
	{18,  300,  T1_PLL(1, 50, 1)},
	{19,  312,  T1_PLL(1, 52, 1)},
	{20,  318,  T1_PLL(1, 53, 1)},
	{21,  330,  T1_PLL(1, 55, 1)},
	{22,  342,  T1_PLL(1, 57, 1)},
	{23,  348,  T1_PLL(1, 58, 1)},
	{24,  360,  T1_PLL(1, 60, 1)},
	{25,  372,  T1_PLL(1, 62, 1)},
	{26,  378,  T1_PLL(1, 63, 1)},
	{27,  390,  T1_PLL(1, 65, 1)},
	{28,  402,  T1_PLL(1, 67, 1)},
	{29,  408,  T1_PLL(1, 68, 1)},
	{30,  420,  T1_PLL(1, 70, 1)},
	{31,  432,  T1_PLL(1, 72, 1)},
	{32,  438,  T1_PLL(1, 73, 1)},
	{33,  450,  T1_PLL(1, 75, 1)},
	{34,  462,  T1_PLL(1, 77, 1)},
	{35,  468,  T1_PLL(1, 78, 1)},
	{36,  480,  T1_PLL(1, 80, 1)},
	{37,  492,  T1_PLL(1, 82, 1)},
	{38,  498,  T1_PLL(1, 83, 1)},
	{39,  510,  T1_PLL(1, 85, 1)},
	{40,  522,  T1_PLL(1, 87, 1)},
	{41,  528,  T1_PLL(1, 88, 1)},
	{42,  540,  T1_PLL(1, 90, 1)},
	{43,  552,  T1_PLL(1, 92, 1)},
	{44,  558,  T1_PLL(1, 93, 1)},
	{45,  570,  T1_PLL(1, 95, 1)},
	{46,  582,  T1_PLL(1, 97, 1)},
	{47,  588,  T1_PLL(1, 98, 1)},
	{48,  600,  T1_PLL(1, 50, 0)},
	{49,  612,  T1_PLL(1, 51, 0)},
	{50,  624,  T1_PLL(1, 52, 0)},
	{51,  630,  T1_PLL(2, 105,0)},
	{52,  636,  T1_PLL(1, 53, 0)},
	{53,  648,  T1_PLL(1, 54, 0)},
	{54,  660,  T1_PLL(1, 55, 0)},
	{55,  672,  T1_PLL(1, 56, 0)},
	{56,  684,  T1_PLL(1, 57, 0)},
	{57,  690,  T1_PLL(2, 115,0)},
	{58,  696,  T1_PLL(1, 58, 0)},
	{59,  708,  T1_PLL(1, 59, 0)},
	{60,  720,  T1_PLL(1, 60, 0)},
	{61,  732,  T1_PLL(1, 61, 0)},
	{62,  744,  T1_PLL(1, 62, 0)},
	{63,  750,  T1_PLL(2, 125,0)},
	{64,  756,  T1_PLL(1, 63, 0)},
	{65,  768,  T1_PLL(1, 64, 0)},
	{66,  780,  T1_PLL(1, 65, 0)},
	{67,  792,  T1_PLL(1, 66, 0)},
	{68,  804,  T1_PLL(1, 67, 0)},
	{69,  810,  T1_PLL(2, 135,0)},
	{70,  816,  T1_PLL(1, 68, 0)},
	{71,  828,  T1_PLL(1, 69, 0)},
	{72,  840,  T1_PLL(1, 70, 0)},
	{73,  852,  T1_PLL(1, 71, 0)},
	{74,  864,  T1_PLL(1, 72, 0)},
	{75,  870,  T1_PLL(2, 145,0)},
	{76,  876,  T1_PLL(1, 73, 0)},
	{77,  888,  T1_PLL(1, 74, 0)},
	{78,  900,  T1_PLL(1, 75, 0)},
	{79,  912,  T1_PLL(1, 76, 0)},
	{80,  924,  T1_PLL(1, 77, 0)},
	{81,  930,  T1_PLL(2, 155,0)},
	{82,  936,  T1_PLL(1, 78, 0)},
	{83,  948,  T1_PLL(1, 79, 0)},
	{84,  960,  T1_PLL(1, 80, 0)},
	{85,  972,  T1_PLL(1, 81, 0)},
	{86,  984,  T1_PLL(1, 82, 0)},
	{87,  990,  T1_PLL(2, 165,0)},
	{88,  996,  T1_PLL(1, 83, 0)},
	{89,  1008, T1_PLL(1, 84, 0)},
	{90,  1020, T1_PLL(1, 85, 0)},
	{91,  1032, T1_PLL(1, 86, 0)},
	{92,  1044, T1_PLL(1, 87, 0)},
	{93,  1050, T1_PLL(2, 175,0)},
	{94,  1056, T1_PLL(1, 88, 0)},
	{95,  1068, T1_PLL(1, 89, 0)},
	{96,  1080, T1_PLL(1, 90, 0)},
	{97,  1092, T1_PLL(1, 91, 0)},
	{98,  1104, T1_PLL(1, 92, 0)},
	{99,  1110, T1_PLL(2, 185,0)},
	{100, 1116, T1_PLL(1, 93, 0)},
	{101, 1128, T1_PLL(1, 94, 0)},
	{102, 1140, T1_PLL(1, 95, 0)},
	{103, 1152, T1_PLL(1, 96, 0)},
	{104, 1164, T1_PLL(1, 97, 0)},
	{105, 1170, T1_PLL(2, 195,0)},
	{106, 1176, T1_PLL(1, 98, 0)},
	{107, 1188, T1_PLL(1, 99, 0)},
	{108, 1200, T1_PLL(1, 100,0)},
	{109, 1212, T1_PLL(1, 101,0)},
	{110, 1224, T1_PLL(1, 102,0)},
	{111, 1236, T1_PLL(1, 103,0)},
	{112, 1248, T1_PLL(1, 104,0)},
	{113, 1260, T1_PLL(1, 105,0)},
	{114, 1272, T1_PLL(1, 106,0)},
	{115, 1284, T1_PLL(1, 107,0)},
	{116, 1296, T1_PLL(1, 108,0)},
	{117, 1308, T1_PLL(1, 109,0)},
	{118, 1320, T1_PLL(2, 110,0)},
	{119, 1332, T1_PLL(1, 111,0)},
	{120, 1344, T1_PLL(1, 112,0)},
	{121, 1356, T1_PLL(1, 113,0)},
	{122, 1368, T1_PLL(1, 114,0)},
	{123, 1380, T1_PLL(1, 115,0)},
	{124, 1392, T1_PLL(2, 116,0)},
	{125, 1404, T1_PLL(1, 117,0)},
	{126, 1416, T1_PLL(1, 118,0)},
	{127, 1428, T1_PLL(1, 119,0)},
	{128, 1440, T1_PLL(1, 120,0)},
	{129, 1452, T1_PLL(1, 121,0)},
	{130, 1464, T1_PLL(1, 122,0)},
	{131, 1476, T1_PLL(1, 123,0)},
	{132, 1488, T1_PLL(1, 124,0)},
	{133, 1500, T1_PLL(1, 125,0)},
	{134, 1512, T1_PLL(1, 126,0)},
	{135, 1524, T1_PLL(1, 127,0)},
	{136, 1536, T1_PLL(1, 128,0)},
	{137, 1548, T1_PLL(1, 129,0)},
	{138, 1560, T1_PLL(1, 130,0)},
	{139, 1572, T1_PLL(1, 131,0)},
	{140, 1584, T1_PLL(1, 132,0)},
	{141, 1596, T1_PLL(1, 133,0)}
};
bool T1_SetT1PLLClock(int chain_id, int chip_id, int pllClkIdx)
{
	uint8_t temp_reg[REG_LENGTH];

	memcpy(temp_reg, default_reg[pllClkIdx], REG_LENGTH);
	if(!mcompat_cmd_write_register(chain_id, chip_id, temp_reg, REG_LENGTH)) 
    {
		return false;
	}

    usleep(150000);
	return true;
}

bool set_config_prepll_test(int chain_id)
{
    int i;
    unsigned char temp_reg[REG_LENGTH] = {0};
    for(i=0; i < 88; i++)       // PLL: 1000MHz
    {
		memcpy(temp_reg, default_reg[i], REG_LENGTH);
		if (!T1_SetT1PLLClock(chain_id, 0, i)) {
			write_ageing_log("    ERROR: set default PLL fail\n");
			return false;
		}
	}

    return true;
}

static int find_vid(int chain_id, int vid_start, int volt_target)
{  
    int real_chip_num = 0;
    int retries = 0;
    char text[256] = {0};

    int i = 0;
    unsigned char spi_tx[2] = {0};
    unsigned char spi_rx[4] = {0};

    sprintf(text, "start vid calibration for chain%d:\n", chain_id);
    write_ageing_log(text);

poweron_retry:       
    if(retries >= PWON_TYPE_MAX)
        return vid_start;

    /* step1: 上电流程 */
    chain_power_on(chain_id, retries);
    retries++;

    if(!mcompat_set_spi_speed(chain_id, SPI_SPEED_1562K))
    {
        sprintf(text, "    ERROR: failed to set spi speed for chain%d\n", chain_id);
        write_ageing_log(text);
        goto poweron_retry;
    }
    usleep(500000);
    
    if(!mcompat_cmd_reset(chain_id, 0x0, spi_tx, spi_rx))
    {
        sprintf(text, "    ERROR: failed to reset chain%d\n", chain_id);
        write_ageing_log(text);
        goto poweron_retry;
    }
    usleep(200000);

    real_chip_num = mcompat_cmd_bist_start(chain_id, 0x0);
    if(real_chip_num <= 0 || real_chip_num > MCOMPAT_CONFIG_MAX_CHIP_NUM)
    {
        sprintf(text, "    ERROR: failed to get chip num for chain%d\n", chain_id);
        write_ageing_log(text);
        goto poweron_retry;
    }
    usleep(200000);

    // pll init
    if(!set_config_prepll_test(chain_id))
    {
        sprintf(text, "    ERROR: failed to init pll for chain%d\n", chain_id);
        write_ageing_log(text);
        goto poweron_retry;
    }

    return mcompat_find_chain_vid(chain_id, real_chip_num, vid_start, volt_target);
}

static void chain_power_on(int chain_id, int type)
{
    switch(type)
    {
    case PWON_TYPE_1:
        write_ageing_log("    encore first mode\n");
        mcompat_set_reset(chain_id, 1);
        sleep(1);
        mcompat_set_power_en(chain_id, 1);
        sleep(1);
        mcompat_set_reset(chain_id, 0);
        sleep(1);
        mcompat_set_start_en(chain_id, 1);
        sleep(1);
        mcompat_set_reset(chain_id, 1);
        sleep(1);
        break;
    case PWON_TYPE_2:
        write_ageing_log("    encore second mode\n");
        mcompat_set_start_en(chain_id, 1);
        mcompat_set_reset(chain_id, 1);
        sleep(1);
        mcompat_set_power_en(chain_id, 1);
        sleep(1);
        mcompat_set_start_en(chain_id, 0);
        mcompat_set_reset(chain_id, 0);
        sleep(1);
        mcompat_set_start_en(chain_id, 1);
        mcompat_set_reset(chain_id, 1);
        sleep(1);
        break;
    default:
        break;
    }
}
