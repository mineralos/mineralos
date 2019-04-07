#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "drv_hub.h"

#ifdef SYSTEM_LINUX
#include <sys/mman.h>
#include "mcompat_config.h"
#include "mcompat_drv.h"
#include "mcompat_lib.h"
#endif


typedef struct HUB_DEV_TAG {
    volatile uint8_t *vir_base;
    uint32_t          phy_addr;
    uint32_t          mem_size;
    const char       *name;
} HUB_DEV_T;

static HUB_DEV_T s_dev_list[] = {
    {NULL, 0x43C30000, 0x2000, "spi"},
    {NULL, 0x43C00000, 0x1000, "peripheral"},
    //{NULL, 0x43C32000, 0x1000, "sha256"},
    {NULL, 0xF8000000, 0x1000, "zynq"},
};

#define INDEX_SPI           0
#define INDEX_PERIPHERAL    1
//#define INDEX_SHA256        2
#define INDEX_ZYNQ          2

pthread_mutex_t s_led_lock;

#ifdef SYSTEM_LINUX
static void hub_hardware_init(void)
{
    int fd = 0;
    int i = 0;
    int iMax = sizeof(s_dev_list) / sizeof(s_dev_list[0]);

    mcompat_log(MCOMPAT_LOG_INFO, "max range: 0x%x.\n", PAGE_SIZE);

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(-1 == fd)
    {
        mcompat_log_err("open /dev/mem:");
        return;
    } 
    
    mcompat_log(MCOMPAT_LOG_INFO, "total: %d dev will mmap.\n", iMax);
    for(i = 0; i < iMax; i++)
    {
        s_dev_list[i].vir_base = mmap(NULL, s_dev_list[i].mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, s_dev_list[i].phy_addr);
        if(MAP_FAILED == s_dev_list[i].vir_base)
        {
            close(fd);
            mcompat_log_err("mmap %s:phy:0x%08x => vir:%p size:0x%x fail.\n", s_dev_list[i].name, s_dev_list[i].phy_addr, s_dev_list[i].vir_base, s_dev_list[i].mem_size);
            return;
        }

        mcompat_log(MCOMPAT_LOG_INFO, "mmap %s:phy:0x%08x => vir:%p size:0x%x ok.\n", s_dev_list[i].name, s_dev_list[i].phy_addr, s_dev_list[i].vir_base, s_dev_list[i].mem_size);
    }

    mcompat_log(MCOMPAT_LOG_INFO, "total: %d dev mmap done.\n", iMax);

    close(fd);
}

static void hub_hardware_deinit(void)
{
    int i = 0;
    int iMax = sizeof(s_dev_list) / sizeof(s_dev_list[0]);

    for(i = 0; i < iMax; i++)
    {
        munmap((void *)s_dev_list[i].vir_base, s_dev_list[i].mem_size);
        mcompat_log(MCOMPAT_LOG_INFO, "unmap %s:vir:%p.\n", s_dev_list[i].name, s_dev_list[i].vir_base);
    }
}


void hub_init(void)
{
    hub_hardware_init();
}

void hub_deinit(void)
{
    hub_hardware_deinit();
}
#endif


// for peripheral ip
void Xil_Peripheral_Out32(uint32_t phyaddr, uint32_t val)
{
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_PERIPHERAL].mem_size -1));
    pgoffset = phyaddr;
    
#ifdef SYSTEM_LINUX
    // for software team
    *(volatile uint32_t *)(s_dev_list[INDEX_PERIPHERAL].vir_base + pgoffset) = val;
#else
    // for digit team
    *(volatile uint32_t *)(s_dev_list[INDEX_PERIPHERAL].phy_addr + pgoffset) = val;
#endif
}

int Xil_Peripheral_In32(uint32_t phyaddr)
{
    uint32_t val;
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_PERIPHERAL].mem_size -1));
    pgoffset = phyaddr;

#ifdef SYSTEM_LINUX
    // for software team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_PERIPHERAL].vir_base + pgoffset);
#else
    // for digit team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_PERIPHERAL].phy_addr + pgoffset);
#endif

    return val;
}

// for spi ip
void Xil_SPI_Out32(uint32_t phyaddr, uint32_t val)
{
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_SPI].mem_size -1));

#ifdef SYSTEM_LINUX
    // for software team
    *(volatile uint32_t *)(s_dev_list[INDEX_SPI].vir_base + pgoffset) = val;
#else
    // for digit team
    *(volatile uint32_t *)(s_dev_list[INDEX_SPI].phy_addr + pgoffset) = val;
#endif
}

int Xil_SPI_In32(uint32_t phyaddr)
{
    uint32_t val;
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_SPI].mem_size -1));

#ifdef SYSTEM_LINUX
    // for software team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_SPI].vir_base + pgoffset);
#else
    // for digit team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_SPI].phy_addr + pgoffset);
#endif

    return val;
}

#if 0
/* Sha256 */
void Xil_SHA256_Out32(uint32_t phyaddr, uint32_t val)
{
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_SHA256].mem_size -1));

#ifdef SYSTEM_LINUX
    // for software team
    *(volatile uint32_t *)(s_dev_list[INDEX_SHA256].vir_base + pgoffset) = val;
#else
    // for digit team
    *(volatile uint32_t *)(s_dev_list[INDEX_SHA256].phy_addr + pgoffset) = val;
#endif
}

uint32_t Xil_SHA256_In32(uint32_t phyaddr)
{
    uint32_t val;
    uint32_t pgoffset = phyaddr & ((uint32_t)(s_dev_list[INDEX_SHA256].mem_size -1));

#ifdef SYSTEM_LINUX
    // for software team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_SHA256].vir_base + pgoffset);
#else
    // for digit team
    val = *(volatile uint32_t *)(s_dev_list[INDEX_SHA256].phy_addr + pgoffset);
#endif

    return val;
}

void Xil_SHA256(unsigned char chain_id, uint32_t *rhash, uint32_t *state, uint32_t *wdata)
{
    int i = 0;

    uint32_t start_reg_offset = 0x0400;
    uint32_t ready_reg_offset = 0x0404;
    /* 中断未使用 */
    /* uint32_t intr_reg_offset  = 0x0408; */
    uint32_t reset_reg_offset = 0x040c;

    uint32_t sha0_base_offset = 0x0000;
    uint32_t state_offset     = 0x0000;
    uint32_t wdata_offset     = 0x0020;
    uint32_t rhash_offset     = 0x0060;
    uint32_t offset_between2_sha = 0x0080;

    uint32_t state_reg_nums = 8;
    uint32_t wdata_reg_nums = 16;
    uint32_t rhash_reg_nums = 8;

    uint32_t phyaddr = 0;
    uint32_t val = 0;
    uint32_t bit_pos = 0;

    /* step1: release resetn */
    val = Xil_SHA256_In32(reset_reg_offset);
    bit_pos = 1 << chain_id;
    val = val | bit_pos;
    Xil_SHA256_Out32(reset_reg_offset, val);

    /* step2: write data */
    /* state */
    for(i = 0; i < state_reg_nums; i++)
    {
        phyaddr = sha0_base_offset
            + chain_id * offset_between2_sha
            + state_offset
            + i * 4;
        val = state[i];
        Xil_SHA256_Out32(phyaddr, val);
    }
    /* wdata */
    for(i = 0; i < wdata_reg_nums; i++)
    {
        phyaddr = sha0_base_offset
            + chain_id * offset_between2_sha
            + wdata_offset
            + i * 4;
        val = wdata[i];
        Xil_SHA256_Out32(phyaddr, val);
    }

    /* step3: start */
    val = Xil_SHA256_In32(start_reg_offset);
    bit_pos = 1 << chain_id;
    val = val | bit_pos;
    Xil_SHA256_Out32(start_reg_offset, val);

    /* step4: wait done */
    do {
        val = Xil_SHA256_In32(ready_reg_offset);
        bit_pos = 1 << chain_id;
        if(bit_pos & val)
        {
            break;
        }
        else /* TODO:加入微量延迟 可以优化cpu负载 */
        {
        }
    } while(1);

    /* step5: read out */
    for(i = 0; i < rhash_reg_nums; i++)
    {
        phyaddr = sha0_base_offset
            + chain_id * offset_between2_sha
            + rhash_offset
            + i * 4;
        rhash[i] = Xil_SHA256_In32(phyaddr);
    }

    /* step6: reset again,防止硬件未做保护 */
    val = Xil_SHA256_In32(reset_reg_offset);
    bit_pos = 1 << chain_id;
    val = val & ~bit_pos;
    Xil_SHA256_Out32(reset_reg_offset, val);

    return;
}
#endif


// ---------------------------------------------------------------------------
// For check status
// ---------------------------------------------------------------------------

int clear_wait_st_idle(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t data_buf;
    uint32_t cmd_status = 0;

    for(i=0; i<timeout_us; i++){
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        if((cmd_status&0xFF000000)==0x00000000) {
            Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000800);
            usleep(1);
            Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000002);
            Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000000);
            data_buf = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
            if((data_buf&0xFFFF00FF) != 0) {
                mcompat_log(MCOMPAT_LOG_DEBUG, "clear_wait_st_idle SPI status is not cleared: %08x i=%0d \n", data_buf&0xFFFF00FF, i);
            }
            break; // state=0 cmd_done=1
        }
        usleep(1);
    }
    if(i >= timeout_us){
        mcompat_log(MCOMPAT_LOG_WARNING, "clear_wait_st_idle Wait SPI status clear timeout! i=%0d status=%8x \n", i, cmd_status);
        return XST_FAILURE;
    }
    else {
        return XST_SUCCESS;
    }
}


int wait_cmd_done(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t timeout;
    uint32_t cmd_status = 0;

    timeout = timeout_us/10;
    for(i = 0; i < timeout; i++){ // polling cmd_done
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        mcompat_log(MCOMPAT_LOG_DEBUG, "Read SPI CMD_CTRL_REG2_ADDR: %08x \n", cmd_status);
        //getchar(); // for debug
        if((cmd_status&0xF0000000) != 0){ // spi fsm not idle
        }
        if((cmd_status&0x00000001) == 1){
            mcompat_log(MCOMPAT_LOG_DEBUG, "CMD  done, status: %08x! \n", cmd_status);
            break;
        }
        usleep(10);
    }
    if(i >= timeout){
        mcompat_log(MCOMPAT_LOG_WARNING, "SPI polling cmd done timeout! i=%0d \n", i);
        return XST_FAILURE;
    }
    else {
        return XST_SUCCESS;
    }
}

int wait_phy_idle(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t cmd_status = 0;

    for(i=0; i<timeout_us; i++){
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        if((cmd_status&0x00000040)==0x00000000) {
            cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
            break; // state=0 cmd_done=1
        }
        usleep(1);
    }
    if(i >= timeout_us){
        mcompat_log(MCOMPAT_LOG_WARNING, "Wait SPI status clear timeout! i=%0d status=%8x \n", i, cmd_status);
        return XST_FAILURE;
    }
    else {
        return XST_SUCCESS;
    }
}

int wait_spi_idle(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t cmd_status = 0;

    for(i=0; i<timeout_us; i+=20){
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        if((cmd_status&0xFF000040)==0) return XST_SUCCESS;
        usleep(20);
    }

    mcompat_log(MCOMPAT_LOG_DEBUG, "Wait SPI(%0d) idle timeout! time=%0d us, status=%8x \n", spi_id, timeout_us, cmd_status);

    return XST_FAILURE;
}

int wait_write_buf_empty(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t cmd_status = 0;

    for(i=0; i<timeout_us; i+=10){
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        if((cmd_status&0x03000000)==0) return XST_SUCCESS;
        usleep(10); // polling interval
    }

    //printf("Wait command write queue empty timeout!spi=%0d i=%0d status=%8x \n",spi_id, i, cmd_status);
    return XST_FAILURE;
}


int check_cmd_status(uint8_t spi_id)
{
    uint32_t cmd_status = 0;

    cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
    if(cmd_status & 0x00000004){
        return XST_CRC_ERROR;
    }

    return XST_SUCCESS;
}


void reset_rx_buffer(uint8_t spi_id)
{
    uint32_t cmd_status = 0;
    uint32_t write_data = 0;

    cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG1_ADDR);
    write_data = cmd_status & (~0x00000004);
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG1_ADDR, write_data);
    usleep(1000);
    write_data = cmd_status | 0x00000004;
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG1_ADDR, write_data);
}

// ---------------------------------------------------------------------------
// Below function is for new SPI design's nonce/receive queue
// ---------------------------------------------------------------------------
void read_rx_buffer(uint8_t spi_id, uint8_t* buf8, uint32_t len_cfg)
{
    uint32_t i;
    uint8_t rx_len;

    rx_len = ((len_cfg & 0x0000FF00) >> 8)*2;

    // Get nonce data
    for(i = 0; i < rx_len; i+=4){
        *(uint32_t*)(buf8+i) = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_READ_REG0_ADDR+i);
    }
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000800); // tell hw one nonce is fetched

}


void fill_tx_buffer(uint8_t spi_id, uint8_t* buf8, uint32_t byte_len)
{
    uint32_t i;

    for(i = 0; i < byte_len; i+=4)
    {
        Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_WRITE_REG01_ADDR+i, *(uint32_t*)(buf8+i));
    }
}


// ---------------------------------------------------------------------------
// Below function is for new SPI design's send queue
// ---------------------------------------------------------------------------

// SPI bypass = 0
int push_one_cmd(uint8_t spi_id, uint8_t* tx_buf8, uint32_t len_cfg, uint32_t last_job)
{
    //uint32_t i;
    uint8_t  byte_len;
    //uint32_t cmd_status;
    uint16_t cmd_header;

    byte_len = (len_cfg >> 24)*2;  // Not include ending zeros
    cmd_header = (tx_buf8[1] << 8) | tx_buf8[0];

    // wait command write queue empty
    if(wait_write_buf_empty(spi_id, 10000) == XST_FAILURE) return XST_FAILURE;

    if(wait_spi_idle(spi_id, 10000) == XST_FAILURE) return XST_FAILURE;


    // write header to buffer
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_WRITE_HEAD_ADDR, cmd_header);


    // write data to buffer
    fill_tx_buffer(spi_id, tx_buf8+2, byte_len-2); // Not include tx


    // send command execution
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG0_ADDR, len_cfg);
    if(last_job)
        Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, (0x00000001 | CHK_CMD | CHK_HY | CHK_LN));
    else
        Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, (0x00000011 | CHK_CMD | CHK_HY | CHK_LN));

    return XST_SUCCESS;
}

/***************************************/
//              interface
/***************************************/

#define ZYNQ_I2C_RESET_CTRL_OFFSET			(0x224)

void hub_i2c_reset()
{
	uint8_t *reg = s_dev_list[INDEX_ZYNQ].vir_base + ZYNQ_I2C_RESET_CTRL_OFFSET;

	*(volatile unsigned int*)reg = 0x1;
    sleep(1);
	*(volatile unsigned int*)reg = 0x0;
	sleep(1);	/* It's necessary to wait for 1 sec after reset */
	mcompat_log(MCOMPAT_LOG_NOTICE, "i2c hw reset done");
}

void hub_spi_reset(uint8_t spi_id)
{
	Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG1_ADDR,0x00000010);
	usleep(1);
	Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG1_ADDR,0x0000001F);
}

extern pthread_mutex_t g_i2c_lock;

int hub_spi_init(uint8_t spi_id, uint8_t chip_num)
{
    uint32_t Status;
    //uint32_t i;

    Status = Xil_SPI_In32(SPI_RESET_REG);

    // reset
    Xil_SPI_Out32(SPI_RESET_REG,(Status & ~(1 << spi_id)));
    usleep(1);

    Status = Xil_SPI_In32(SPI_RESET_REG);

    // release reset
    Xil_SPI_Out32(SPI_RESET_REG,(Status | (1 << spi_id)));

    Status = Xil_SPI_In32(SPI_RESET_REG);

	hub_spi_reset(spi_id);
    // config max chip number
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR,0x00041004|(chip_num<<24));
    // config not check header
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG3_ADDR,0x000F0000 & chip_num);
    // config mask of each interrupt
    Xil_SPI_Out32(MAIN_CFG_REG2_ADDR, 0x00000);

    pthread_mutex_init(&g_i2c_lock, NULL);

    return XST_SUCCESS;
}

void hub_spi_clean_chain(uint32_t spi_id)
{
	uint8_t spi_tx[MCOMPAT_CONFIG_MAX_CMD_LENGTH] = {0};
	uint8_t spi_rx[MCOMPAT_CONFIG_MAX_CMD_LENGTH] = {0};

	spi_tx[0] = CMD_RESET;
	spi_tx[1] = CMD_ADDR_BROADCAST;
	spi_tx[2] = 0xff;
	spi_tx[3] = 0xff;

	do_spi_cmd(spi_id, spi_tx, spi_rx, 0x10001000);

	hub_spi_reset(spi_id);
}

void hub_set_spi_speed(uint8_t spi_id, int select)
{
    uint32_t cfg[] =      {0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200100, 0x00330100};
    //float mcu_spi_clk[] = {0.39062,    0.78125,    1.5625,     3.125,      6.25,       9.96};
    uint32_t read_data;

    read_data = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR);
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR,(read_data&0xFF00FFFF)|cfg[select]);

    read_data = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR);
}


// ----------- send job ------------
int send_job_queue(uint8_t spi_id, uint8_t* tx_buf8, uint8_t* rx_buf8, uint32_t len_cfg, uint32_t last_job)
{
    // push tx data to send buffer and start command
    if(push_one_cmd(spi_id, tx_buf8, len_cfg, last_job) != XST_SUCCESS) return XST_FAILURE;

    // wait all jobs are sent to chip
    if(last_job){
        if(wait_spi_idle(spi_id, 100000) == XST_FAILURE) return XST_FAILURE;
    }

    // Clear status. Not mandatory but suggest
    if(last_job) clear_wait_st_idle(spi_id, 100000);

//  usleep(100);
    return XST_SUCCESS;
}


int send_one_cmd_split(uint8_t spi_id, uint8_t* tx_buf8, uint32_t len_cfg, uint32_t last_job, uint8_t cs_low)
{
    uint32_t  byte_len;
    uint16_t cmd_header;
    uint32_t cfg_reg0;

    byte_len = (len_cfg >> 24)*2;  // Not include ending zeros
    cmd_header = (tx_buf8[1] << 8) | tx_buf8[0];

    // change ext_zero to 0
    cfg_reg0 = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR);
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR,(cfg_reg0&0xFFFFFF00));


    // wait command write queue empty
    if(wait_write_buf_empty(spi_id, 10000) == XST_FAILURE) return XST_FAILURE;


    // write header to buffer
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_WRITE_HEAD_ADDR, cmd_header);


    // write data to buffer
    fill_tx_buffer(spi_id, tx_buf8+2, byte_len-2); // Not include tx


    // send command execution
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG0_ADDR, len_cfg);
    if(last_job)
        Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, (0x00000001 | CHK_HY | CHK_LN | cs_low<<14) );
    else
        Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, (0x00000011 | CHK_HY | CHK_LN | cs_low<<14) );


//    // check send queue full
//    cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
//    if((cmd_status&0x02000000)==0) {
//        DBGERROR("After push one command, send queue is not full!");
//        return XST_FAILURE;
//    }

    // For half command application, can not use queue
    wait_cmd_done(spi_id, 10000);


    // change back ext_zero
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG0_ADDR,cfg_reg0);


    return XST_SUCCESS;

}


bool rece_queue_ready_check(uint8_t spi_id, uint32_t len, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t cmd_status;
	uint32_t timeout = timeout_us / 1000 + 1;

	for(i = 0; i <= timeout; i++){
		cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
		if (((cmd_status&0x00000010)==0x00000010) && ((((cmd_status&0x0000FF00)>>8)>=(len)))) return true; // wait nonce_ready = 1, cmd_done = 1
		//        if (((spi_tr.cmd_status&0x0000FF00)>>8)>=(len)) return true;
		//        if ((spi_tr.cmd_status&0x00000010)==0x00000010) return true;
		usleep(1000); // usleep(10);
	}

    return false;
}

// Check receive queue empty. true: empty
bool rece_queue_empty_check(uint8_t spi_id, uint32_t timeout_us)
{
    uint32_t i;
    uint32_t cmd_status;

    for(i=0; i<timeout_us/10; i++){
        cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
        if((cmd_status & 0x0000ff10)==0x00000000) {
            return true; // wait nonce_ready = 0, cmd_done = 1
       }
        usleep(10);
    }

    return false;
}

bool rece_queue_has_nonce(uint8_t spi_id, uint32_t timeout_us)
{
    //uint32_t i;
    uint32_t cmd_status;

    cmd_status = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG2_ADDR);
    if( ((cmd_status&0x0000ff00) >= 0x00000600) && ((cmd_status&0x00000010) == 0x00000010) ) {
        return true; // wait nonce_ready = 0, cmd_done = 1
    }

    return false;
}


void read_nonce_buffer(uint8_t spi_id, uint8_t* buf8, uint32_t len_cfg)
{
    uint32_t i;
    uint8_t rx_len;

    rx_len = (len_cfg & 0x0000FF00) >> 8;

    // Get nonce data
    for(i = 0; i < rx_len*2; i+=4){
        *(uint32_t*)(buf8+i) = Xil_SPI_In32(SPI_BASEADDR_GAP*spi_id+CMD_READ_REG0_ADDR+i);
    }
    /*
    if(buf8[0] == 00)
    {
        dump_spi_last_tr(spi_id);
    }
    */
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000B00); // tell hw one nonce is fetched, keep auto get nonce enable
    usleep(1);
}


int pop_one_rece(uint8_t spi_id, uint8_t* rx_buf8, uint32_t len_cfg)
{
    //uint32_t i;
    uint32_t len = (len_cfg & 0x0000ff00) >> 8;

    // wait receive queue ready
    if(rece_queue_ready_check(spi_id, len, 50000) == false) {
        mcompat_log(MCOMPAT_LOG_WARNING, "Check receive buffer ready timeout! \n");
        return XST_FAILURE;
    }
/*
    if(check_crc_status(spi_id) != XST_SUCCESS) {
        mcompat_log(MCOMPAT_LOG_WARNING, "crc error \n");
        return XST_CRC_ERROR;
    }
*/
    // read back rx data
    read_rx_buffer(spi_id, rx_buf8, len_cfg);

    return XST_SUCCESS;
}


int do_spi_cmd(uint8_t spi_id, uint8_t* tx_buf8, uint8_t* rx_buf8, uint32_t len_cfg)
{
   // reset_rx_buffer(spi_id);

    // push tx data to send buffer and start command
    if(push_one_cmd(spi_id, tx_buf8, len_cfg, 1) != XST_SUCCESS) {
        mcompat_log(MCOMPAT_LOG_ERR, "ERROR - failed to send spi cmd");
        return XST_FAILURE;
    }

    // read back rx data
    if(pop_one_rece(spi_id, rx_buf8, len_cfg) != XST_SUCCESS) {
        mcompat_log(MCOMPAT_LOG_ERR, "ERROR - failed to recv spi data");
        return XST_FAILURE;
    }

    if((tx_buf8[0] & 0x0f) != (rx_buf8[0] & 0x0f)) {
        mcompat_log(MCOMPAT_LOG_ERR, "ERROR - spi recv data invalid: %02X|%02X", tx_buf8[0], rx_buf8[0]);
        return XST_FAILURE;
    }

    // Clear status. Not mandatory but suggest
    clear_wait_st_idle(spi_id, 200);

    return XST_SUCCESS;
}


void enable_auto_cmd0a(uint8_t spi_id, uint32_t threshold, uint32_t msb, uint32_t lsb, uint32_t large_en, uint32_t mode )//mode : 1 only cmd0a;0 cmd08 follows cmd0a
{
    uint32_t val;
	Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000000 | CHK_CMD);
    val = ((msb << 24) & 0xff000000) | ((lsb << 16) & 0xff0000) | ((mode & 0x1) << 14) | ((large_en & 0x1) << 13) | (0x1 << 12) | ( threshold & 0xfff );
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+AUTO_CMD0A_REG0_ADDR, val);
}

void disable_auto_cmd0a(uint8_t spi_id, uint32_t threshold, uint32_t msb, uint32_t lsb, uint32_t large_en, uint32_t mode )//mode : 1 only cmd0a;0 cmd08 follows cmd0a
{
    uint32_t val;
    val = ((msb << 24) & 0xff000000) | ((lsb << 16) & 0xff0000) | ((mode & 0x1) << 14) | ((large_en & 0x1) << 13) | (0x0 << 12) | ( threshold & 0xfff );
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+AUTO_CMD0A_REG0_ADDR, val);
}


int enable_auto_nonce(uint8_t spi_id, uint16_t cmd08_cmd, uint32_t len_cfg)
{
    uint8_t send_buf8[12] = {0};

    // wait all previous command done
    //clear_wait_st_idle(spi_id, 1000000);
    wait_spi_idle(spi_id, 10000);

    // set auto get nonce
    //for(i=0; i<spi_tr.tx_len; i++){spi_tr.tx_buf[i] = 0;} // clear tx_buf variable for debug print
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_WRITE_HEAD_ADDR, REORDER16(cmd08_cmd));
    fill_tx_buffer(spi_id, send_buf8, 10);
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+MAIN_CFG_REG3_ADDR, 0x000000ff); // auto cmd08 gap
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG0_ADDR, len_cfg);
    //Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000002); // clear status
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000300 | CHK_CMD); // cmd08 do

    return XST_SUCCESS;
}

int disable_auto_nonce(uint8_t spi_id)
{
    Xil_SPI_Out32(SPI_BASEADDR_GAP*spi_id+CMD_CTRL_REG1_ADDR, 0x00000000); // disable auto get nonce

    // wait command done
    return (clear_wait_st_idle(spi_id, 1000000));

}

void hub_set_power_en_i2c(uint8_t chain_id,int value);
// 3.3v GPIO output
void hub_set_power_en(uint8_t chain_id, int value)
{
    uint32_t reg_val;

    // special power en mode for i2c power
    if (MCOMPAT_LIB_VID_I2C_TYPE == g_vid_type) {
		hub_set_power_en_i2c(chain_id, value);
		sleep(3);
	}

    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET, (reg_val & (~(0x1 << (18 + chain_id)))) | ((value & 0x1) << (18 + chain_id)));
    //reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4);
    //Xil_Peripheral_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4, (reg_val & 0xfffeffff) | ((value & 0x1) << 16));

}

// 1.8v GPIO output
void hub_set_start_en(uint8_t chain_id, int value)
{
    uint32_t reg_val;
    
    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET, (reg_val & (~(0x1 << chain_id))) | ((value & 0x1) << chain_id));
    //reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4);
    //Xil_Peripheral_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4, (reg_val & 0xfffbffff) | ((value & 0x1) << 18));
}

// 1.8v GPIO output
void hub_set_reset(uint8_t chain_id, int value)
{
    uint32_t reg_val;
    
    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG8_OFFSET, (reg_val & (~(0x1 << (9 + chain_id)))) | ((value & 0x1) << (9 + chain_id)));
    //reg_val = Xil_Peripheral_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4);
    //Xil_Peripheral_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + chain_id*4, (reg_val & 0xfffdffff) | ((value & 0x1) << 17));
}

void hub_set_led(uint8_t chain_id, int mode)
{
    uint32_t reg_val;

    pthread_mutex_lock(&s_led_lock);

    if(mode == LED_ON){
        reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << chain_id)) ) | ((LED_ON & 0x1) << chain_id));
    }
    else if(mode == LED_OFF){
        reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << chain_id)) ) | ((LED_OFF & 0x1) << chain_id));
    }

    pthread_mutex_unlock(&s_led_lock);
}

int hub_get_plug(uint8_t chain_id)
{
    uint32_t reg_val;

    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG10_OFFSET);
    return ((reg_val >> chain_id) & 0x01);
}


#ifdef SYSTEM_LINUX
static int set_warn(int spi_id)
{
    mcompat_set_power_en(spi_id, 0);
    sleep(1);
    mcompat_set_reset(spi_id, 0);
    mcompat_set_start_en(spi_id, 0);

    do
    {
        mcompat_set_led(spi_id, 1);
        sleep(1);
        mcompat_set_led(spi_id, 0);
        sleep(1);
    }while(1);

    return 0;
}


uint32_t hub_get_hitemp_stat(uint8_t chain_id,mcompat_temp_s *temp_ctrl)
{
    bool over_temp = false;
    uint32_t reg_val;
    uint32_t tmp_val;

    reg_val = Xil_SPI_In32(SPI_BASEADDR_GAP*chain_id+AUTO_CMD0A_REG3_ADDR);

    tmp_val = ((reg_val ) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_highest[0] = tmp_val;

    if((temp_ctrl->temp_highest[0]) &&(temp_ctrl->temp_highest[0] < g_dangerous_temp))
        over_temp = true;

    tmp_val = ((reg_val >> 10) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val >> 10) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_highest[1] = tmp_val;

    if((temp_ctrl->temp_highest[1]) &&(temp_ctrl->temp_highest[1] < g_dangerous_temp))
        over_temp = true;

    tmp_val = ((reg_val >> 20) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val >> 20) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_highest[2] = (reg_val >> 20 ) & 0x3ff;

    if((temp_ctrl->temp_highest[2]) &&(temp_ctrl->temp_highest[2] < g_dangerous_temp))
        over_temp = true;

    if(over_temp == true)
        set_warn(chain_id);
 
    mcompat_log(MCOMPAT_LOG_INFO,"chain %d,Hi: %d,%d,%d\n",chain_id,temp_ctrl->temp_highest[0],temp_ctrl->temp_highest[1],temp_ctrl->temp_highest[2]);
    return 0;
}

uint32_t hub_get_lotemp_stat(uint8_t chain_id,mcompat_temp_s *temp_ctrl)
{
    uint32_t reg_val;
    uint32_t tmp_val;

    reg_val =  Xil_SPI_In32(SPI_BASEADDR_GAP*chain_id+AUTO_CMD0A_REG2_ADDR);


    tmp_val = ((reg_val) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_lowest[0] = tmp_val;

    tmp_val = ((reg_val >> 10) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val >> 10) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_lowest[1] = tmp_val;


    tmp_val = ((reg_val >> 20) & 0x3ff) < g_temp_hi_thr ? 0x0:((reg_val >> 20) & 0x3ff);
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->temp_lowest[2] = tmp_val;
    mcompat_log(MCOMPAT_LOG_INFO,"chain %d,lo: %d,%d,%d\n",chain_id,temp_ctrl->temp_lowest[0],temp_ctrl->temp_lowest[1],temp_ctrl->temp_lowest[2]);

    return 0;
}

uint32_t hub_get_avgtemp_stat(uint8_t chain_id,mcompat_temp_s *temp_ctrl)
{
    uint32_t reg_val;
    uint32_t tmp_val;

    reg_val =  Xil_SPI_In32(SPI_BASEADDR_GAP*chain_id+AUTO_CMD0A_REG4_ADDR);

    tmp_val = 2 * ((reg_val ) & 0xffff) / g_chip_num;


    tmp_val = tmp_val < g_temp_hi_thr ? 0x0:tmp_val;
    tmp_val = tmp_val > g_temp_lo_thr ? 0x0:tmp_val;
    temp_ctrl->final_temp_avg = tmp_val;
    mcompat_log(MCOMPAT_LOG_INFO,"chain %d,avg: %d\n",chain_id,temp_ctrl->final_temp_avg);

    return 0;
}
#endif


void hub_set_pwm(uint8_t fan_id, int frequency, int duty)
{
#if 0
    int duty_driver = 0;
    unsigned int value;
    duty_driver = frequency / 100 * duty;

    value = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG16_OFFSET + fan_id*8);
    value = value | (1<<fan_id);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG2_OFFSET, value);

    //Xil_Fans_Out32(XPAR_FANS_CTRL_0_S00_AXI_BASEADDR + FANS_CTRL_S00_AXI_SLV_REG1_OFFSET + fan_id*8, frequency);
    //Xil_Fans_Out32(XPAR_FANS_CTRL_0_S00_AXI_BASEADDR + FANS_CTRL_S00_AXI_SLV_REG0_OFFSET + fan_id*8, duty_driver | (0x1 << 31));
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG16_OFFSET + fan_id*8, frequency);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG17_OFFSET + fan_id*8, duty_driver | (0x1 << 31));
#else
    int duty_driver = 0;
    uint32_t reg_val;
    duty_driver = frequency / 100 * duty;

    mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d: fan_id %d, freq: %d duty: %d.\n", __FILE__, __LINE__, fan_id, frequency, duty);

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG16_OFFSET + fan_id*8, frequency);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG17_OFFSET + fan_id*8, duty_driver);
    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG2_OFFSET);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG2_OFFSET, (reg_val & (~(0x1 << fan_id))) | (0x1 << fan_id));
#endif
}

int hub_get_button(void)
{
    uint32_t reg_val;

    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG10_OFFSET);
    return (((reg_val >> 16) & 0x1));
}

void hub_set_green_led(int mode)
{
    uint32_t reg_val,SetRedRegValue;

    pthread_mutex_lock(&s_led_lock);

    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET);

    if(LED_ON == mode)
        SetRedRegValue = reg_val | 0x200;
    else
        SetRedRegValue = reg_val & (~0x200);

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET,SetRedRegValue);

    pthread_mutex_unlock(&s_led_lock);
}

void hub_set_red_led(int mode)
{
    uint32_t reg_val,SetRedRegValue;

    pthread_mutex_lock(&s_led_lock);

    reg_val = Xil_Peripheral_In32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET);

    if(LED_ON == mode)
        SetRedRegValue = reg_val | 0x400;
    else
        SetRedRegValue = reg_val & (~0x400);

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG9_OFFSET,SetRedRegValue);

    pthread_mutex_unlock(&s_led_lock);
}

void init_hub_gpio(void)
{
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG1_OFFSET, 0);
    sleep(1);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG1_OFFSET, 3);

    pthread_mutex_init(&s_led_lock, NULL);
}

