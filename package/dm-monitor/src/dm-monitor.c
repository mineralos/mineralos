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

#include "mcompat_drv.h"
#include "mcompat_lib.h"
#include "dm-stats.h"
#include "dm-socket-server.h"

#define DM_MONITOR_INDENT           ("dm_monitor")
#define DM_MONITOR_GET_DHCP_CMD     ("cat /config/network/25-wired.network | sed -n '/^DHCP/p' | awk -F'=' '{print $2}'")
#define DM_RESET_CONF_CMD           ("rm -rf /config/*; /bin/systemctl reboot")
#define DM_RUN_SELFTEST_CMD         ("/bin/dm-selftest")

#define DM_MONITOR_NET_DHCP         (0)
#define DM_MONITOR_NET_STATIC       (1)
#define DM_MONITOR_NET_ERR          (-1)

#define DM_LED_GREEN                (0x1)
#define DM_LED_RED                  (0x2)
#define DM_LED_ON                   (0)
#define DM_LED_OFF                  (1)
#define DM_QUICK_FLASH_US           (250 * 1000)

#define DM_SEND_IP_ADDR             (INADDR_BROADCAST)

#define DM_INTERFACE_PATH           ("/config/network/25-wired.network")
#define DM_STATIC_IP_ADDR           ("192.168.1.254")
#define DM_NETMASK                  ("24")
#define DM_GATEWAY                  ("192.168.1.1")
#define DM_DNS1                     ("8.8.8.8")
#define DM_DNS2                     ("114.114.114.114")

#define XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR 0x0
#define VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET 36
#define VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG10_OFFSET 40
#define PAGE_SIZE   ((size_t)getpagesize())*2
#define PAGE_MASK   ((uint32_t) (long)~(PAGE_SIZE - 1))
//#define LED_ON                  0
//#define LED_OFF                 1


volatile uint8_t *g_map_peripheral_base;
volatile uint8_t *g_map_base;


/*----------------------------------- 声明区 ----------------------------------*/
typedef struct button_monitor_argv_TAG{
    int  interval;
} button_monitor_argv_T;

typedef struct _c_led_blink
{
    uint32_t led;
    uint32_t blink_cnt;
    uint32_t wait;
    uint32_t loop;
} c_led_blink;

typedef struct _c_chain_stat
{
    int chain_id;
    int pll;
    int vid;
    int local_acc;
    int accept;
    int reject;
    int hwerr;
    double hashrate;
} c_chain_stat;

/********************************** 变量声明区 *********************************/
/* 按键按下时间
 * 0    未按下
 * 其他 按下持续时间 = s_button_press_times * button_monitor_interval
 * */
static int s_button_press_times = 0;
static bool s_dhcp_mode = true;

/********************************** 函数声明区 *********************************/
/*******************************************************************************
 *
 * 函数吊  : usage
 * 描述    : 输出监控进程的使用说明
 * 输入参数: prog_name 程序吊
 * 输出参数: 无
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static bool Xil_Mmap(void);
static void usage(const char *prog_name);
static char *get_mac(char *mac,char *ethn);
static int  get_ip(char *eth, char *ipaddr);
static bool get_dhcp_mode(void);
static void change_net_mode(void);
static void send_a_udp_pkg(struct sockaddr_in *target_addr, const char *msg);
static void revert_default_conf(void);
static int  get_button(void);
static void revert_led(int index);
static void set_red_led(int state);
static void set_green_led(int state);
static void *thread_button_monitor(void *argv);

/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
/*******************************************************************************
 *
 *
 * 函数吊  : main
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 主函数
 *
 * 输入参数: argc - 参数个数
 *           argv - 命令行参数数组
 *
 * 输出参数: 无
 *
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
    int pthread_rst = 0;
    int rst = 0;
    int vol = 0;
    int i = 0;

    pthread_t button_monitor_tid;

    button_monitor_argv_T button_monitor_argv;

    pthread_t cgminer_stats;

    pthread_t socket_srever_tid;

    int button_monitor_interval = 0;

    if(argc < 2)
    {
        usage(argv[0]);
        exit(0);
    }

    button_monitor_interval = atoi(argv[1]);

    rst = sys_platform_init(PLATFORM_ZYNQ_HUB_G19, 0, 8, 80);
    if(!rst)
    {
        mcompat_log_err("sys_platform_init: platform[%d] miner_type[%d] chain_num[%d] chip_num[%d].\n", PLATFORM_ZYNQ_HUB_G19, 0, 8, 80);
        exit(1);
    }

    /* Stats Thread */
    pthread_rst = pthread_create(&cgminer_stats, NULL, thread_get_stats, NULL);
    if(0 != pthread_rst)
    {
        printf("cgminer_stats threat create failled.\n");
    }

    /* socket server for backend Thread */
    pthread_rst = pthread_create(&socket_srever_tid, NULL, thread_socket_server, NULL);
    if(0 != pthread_rst)
    {
        printf("socket server threat create failled.\n");
    }

    /* 线程 button监控 */
    button_monitor_argv.interval = button_monitor_interval;
    pthread_rst = pthread_create(&button_monitor_tid, NULL, thread_button_monitor, &button_monitor_argv);
    if(0 != pthread_rst)
    {
        printf("thread_button_monitor create failled.\n");
    }

    while(1)
    {
#if 0
        //sleep(10); /* 超时值一半的周期喂狗 */
        if(misc_get_vid_type() == MCOMPAT_LIB_VID_I2C_TYPE)
        {
            for(i = 1; i <= 3; i++)
            {
                vol = get_vol_on_i2c(i);
                sleep(1);
            }
        }
#endif
        sleep(5);
    }

    rst = sys_platform_exit();
    if(!rst)
    {
        mcompat_log_err("sys_platform_exit.\n");
    }

    return 0;
}

static void usage(const char *prog_name)
{
    printf("USAGE:\n");
    printf("      %s interval.\n",prog_name);
    printf("      interval is the button func check interval\n");
}

static void *thread_button_monitor(void *argv)
{
    button_monitor_argv_T *button_monitor_argv = (button_monitor_argv_T *)argv;
    int interval = button_monitor_argv->interval;

    int i = 0;
    int button_state = 0;
    struct sockaddr_in target_addr;
    char *msg = "Hello, I'm a Miner.";
    char mac_tmp[30] = {0};
    char sendmsg[100];
    char mac_data[30];
    char ipaddr[40];
    char text[256] = {0};
    memset(mac_data,0,sizeof(char)*30);
    memset(ipaddr,0,sizeof(char)*40);
    memset(sendmsg,0,sizeof(char)*100);

    s_dhcp_mode = get_dhcp_mode();
    printf("dhcp mode: %d\n", s_dhcp_mode);

    s_button_press_times = 0;
    printf("thread_button_monitor start:%d.\n", interval);
    while(1)
    {
        button_state = get_button();

        /* 30秒后自动执行操作 */
        if(s_button_press_times > 15)
            button_state = 1;

        if(0 == button_state)
        {   /* 按键按下时执行的操作 */
            s_button_press_times++; /* 计时 */
            set_red_led(DM_LED_ON);
            set_green_led(DM_LED_ON);
        }
        else
        {   /* 按键弹起时执行的操作 */
            if(s_dhcp_mode)
                revert_led(DM_LED_GREEN);
            else
                set_green_led(DM_LED_ON);

            if(0 == s_button_press_times) /* 未按键 */
            {
                goto CONTINUE_BUTTON_MONITOR;
            }
            else if ((1 <= s_button_press_times) && (s_button_press_times < 4))  /* [1s, 4s) 发IP */
            {
                printf("send udp boardcast for ip address \n");
                set_red_led(DM_LED_OFF);
                set_green_led(DM_LED_OFF);
                memset(&target_addr, 0, sizeof(struct sockaddr_in));
                target_addr.sin_family = AF_INET;
                target_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
                target_addr.sin_port = htons(DM_MONITOR_PORT);

                send_a_udp_pkg(&target_addr, msg);

                if(!get_ip("eth0",ipaddr))
                {
                    printf("get IP error\n");
                }

                get_mac(mac_data,"eth0");
                strncpy(mac_tmp,mac_data,strlen(mac_data));
                strcpy(sendmsg,ipaddr);
                strcat(sendmsg,",");
                strcat(sendmsg,mac_data);

                target_addr.sin_port = htons(14235);
                send_a_udp_pkg(&target_addr, sendmsg);

                /* 快闪2下 */
                for(i = 0; i < 2; i++)
                {
                    revert_led(DM_LED_GREEN);
                    usleep(DM_QUICK_FLASH_US);
                }
            }
            else if ((4 <= s_button_press_times) && (s_button_press_times < 15)) /* [4s, 15s) 恢夊默认配置 */
            {
                printf("reset to default  \n");
                set_red_led(DM_LED_OFF);
                set_green_led(DM_LED_OFF);
                revert_default_conf();

                /* 快闪4下 */
                for(i = 0; i < 4; i++)
                {
                    revert_led(DM_LED_GREEN);
                    usleep(DM_QUICK_FLASH_US);
                }
            }
            else
            {
                printf("switch network mode \n");
                set_red_led(DM_LED_OFF);
                set_green_led(DM_LED_OFF);
                change_net_mode();

                /* 快闪15下 */
                for(i = 0; i < 15; i++)
                {
                    revert_led(DM_LED_GREEN);
                    usleep(DM_QUICK_FLASH_US);
                }
            }

            /* 开始记录下一次按键 */
            s_button_press_times = 0;
        }

        printf("thread_button_monitor new loop:%d.\n", s_button_press_times);

CONTINUE_BUTTON_MONITOR:
        sleep(interval);
    }

    return NULL;
}

static bool get_dhcp_mode(void)
{
    char result[10] = {0};

    mcompat_get_shell_cmd_rst(DM_MONITOR_GET_DHCP_CMD, result, sizeof(result));
    return (strncmp(result, "yes", 3) == 0);
}

static char *get_mac(char *mac,char *ethn)
{
    struct ifreq ifreq;
    int sock;
    char *mac_tmp;
    mac_tmp=(char*)calloc(40,sizeof(char));

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        exit(0);
    }
    strcpy (ifreq.ifr_name, ethn);
    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror ("ioctl");
        exit(0);
    }
    close(sock);
    sprintf (mac_tmp, "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);

    strcpy(mac,mac_tmp);
    free(mac_tmp);
    return mac;
}

static int get_ip(char *eth, char *ipaddr)
{
    int sock_fd;
    struct  sockaddr_in my_addr;
    struct ifreq ifr;

    /**//* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    /**//* Get IP Address */
    strncpy(ifr.ifr_name, eth,20 );
    ifr.ifr_name[IFNAMSIZ-1]='0';

    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        printf(":No Such Device %s/n",eth);
        return -1;
    }

    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
    close(sock_fd);
    return 1;
}

static void revert_led(int index)
{
    /* 记住灯的状态用于闪烁 */
    static int s_green_led_status = 0;
    static int s_red_led_status = 0;

    switch(index)
    {
        case DM_LED_GREEN:
            {
                if(0 == s_green_led_status)
                {
                    set_green_led(1);
                    s_green_led_status = 1;
                }
                else
                {
                    set_green_led(0);
                    s_green_led_status = 0;
                }
                break;
            }

        case DM_LED_RED:
            {
                if(0 == s_red_led_status)
                {
                    set_red_led(1);
                    s_red_led_status = 1;
                }
                else
                {
                    set_red_led(0);
                    s_red_led_status = 0;
                }
                break;
            }

        default:
            {
                printf("invalid led index %d.\n", index);
                break;
            }
    }
}

static void send_a_udp_pkg(struct sockaddr_in *target_addr, const char *msg)
{
    int socket_fd = 0;
    int opt_val = 1; /* 这个值一定要设置，否则可能导致sendto()失败 */

    printf("%s:%s\n", __func__, msg);

    if((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket fail\n");
    }

    setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &opt_val, sizeof(int));

    printf("sendto %s:%u with:%s\n", inet_ntoa(target_addr->sin_addr), ntohs(target_addr->sin_port), msg);
    if(-1 == sendto(socket_fd, msg, strlen(msg), 0, (struct sockaddr *)target_addr, sizeof(struct sockaddr)))
    {
        printf("sendto fail\n");
    }

    close(socket_fd);
}

static void revert_default_conf(void)
{
    const char *cmd = DM_RESET_CONF_CMD;

    printf("%s,%s.\n", __func__, cmd);
    system(cmd);
    sync();
    sync();
    sync();

    s_dhcp_mode = get_dhcp_mode();
}

static void change_net_mode(void)
{
    FILE *fp_interface = NULL;

    printf("%s:", __func__);

    s_dhcp_mode = !get_dhcp_mode();

    fp_interface = fopen(DM_INTERFACE_PATH, "w");
    if(NULL == fp_interface)
    {
        printf("fopen %s fail.\n", DM_INTERFACE_PATH);
        return;
    }

    if(s_dhcp_mode)         /* The current static IP is changed to dynamic IP */
    {
        printf("static ip ===> dhcp.\n");

        fprintf(fp_interface, "[Match]\n");
        fprintf(fp_interface, "Name=eth0\n");
        fprintf(fp_interface, "[Network]\n");
        fprintf(fp_interface, "DHCP=yes\n");
        fprintf(fp_interface, "[DHCP]\n");
        fprintf(fp_interface, "ClientIdentifier=mac\n");
    }
    else                    /* The current dynamic IP is changed to static IP */
    {
        printf("dhcp ===> static ip.\n");

        /* interface */
        fprintf(fp_interface, "[Match]\n");
        fprintf(fp_interface, "Name=eth0\n");
        fprintf(fp_interface, "[Network]\n");
        fprintf(fp_interface, "Address=%s/%s\n", DM_STATIC_IP_ADDR, DM_NETMASK);
        fprintf(fp_interface, "Gateway=%s\n", DM_GATEWAY);
        fprintf(fp_interface, "DNS=%s\n", DM_GATEWAY);
        fprintf(fp_interface, "DNS=%s\n", DM_GATEWAY);
    }
    fclose(fp_interface);

    /* Restart network */
    system("systemctl restart systemd-networkd\n");

    sync();
    sync();
    sync();
}

static int get_button(void)
{
    return mcompat_get_button();
}

static void set_green_led(int state)
{
    mcompat_set_green_led(state);
}

static void set_red_led(int state)
{
    mcompat_set_red_led(state);
}
