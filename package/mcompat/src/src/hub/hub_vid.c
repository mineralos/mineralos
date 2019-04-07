/******************************************************************************
 *
 * 文件名  ： hub_vid.c
 * 负责人  ： pengp
 * 创建日期： 20180307
 * 版本号  ： v1.0
 * 文件描述： hub的vid的实现
 * 版权说明： Copyright (c) 2000-2020
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "mcompat_lib.h"
#include "drv_hub.h"
#include "hub_vid.h"


/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/*  vid, iic, uart */
static int s_vid_map[][3] = {
    {0,  1560, 54},
    {1,  1552, 60},
    {2,  1544, 67},
    {3,  1537, 73},
    {4,  1529, 80},
    {5,  1521, 86},
    {6,  1513, 93},
    {7,  1506, 99},
    {8,  1498, 106},
    {9,  1490, 112},
    {10, 1482, 119},
    {11, 1475, 125},
    {12, 1467, 132},
    {13, 1459, 138},
    {14, 1451, 145},
    {15, 1443, 151},
    {16, 1436, 158},
    {17, 1428, 164},
    {18, 1420, 171},
    {19, 1412, 177},
    {20, 1405, 184},
    {21, 1397, 190},
    {22, 1389, 197},
    {23, 1381, 203},
    {24, 1374, 210},
    {25, 1366, 216},
    {26, 1358, 223},
    {27, 1350, 229},
    {28, 1343, 236},
    {29, 1335, 242},
    {30, 1327, 248},
    {31, 1320, 255},
};
/*
static int s_vid_map[][3] = {
    {0,  1608,  0},
    {1,  1598,  5},
    {2,  1588,  10},
    {3,  1579,  15},
    {4,  1570,  20},
    {5,  1560,  25},
    {6,  1551,  30},
    {7,  1541,  35},
    {8,  1532,  40},
    {9,  1523,  45},
    {10, 1513,  50},
    {11, 1503,  55},
    {12, 1494,  60},
    {13, 1484,  65},
    {14, 1475,  70},
    {15, 1466,  75},
    {16, 1456,  80},
    {17, 1447,  85},
    {18, 1437,  90},
    {19, 1428,  95},
    {20, 1418,  100},
    {21, 1409,  105},
    {22, 1399,  110},
    {23, 1390,  115},
    {24, 1380,  120},
    {25, 1371,  125},
    {26, 1361,  130},
    {27, 1352,  135},
    {28, 1343,  140},
    {29, 1333,  145},
    {30, 1324,  150},
    {31, 1314,  155},
};
*/
/********************************** 函数声明区 *********************************/
static bool hub_set_vid_vid(uint8_t chain_id, int vid);
static bool hub_set_vid_i2c(uint8_t chain_id, int vid);
static bool hub_set_vid_uart(uint8_t chain_id, int vid);
static void hub_set_vid_uart_select(uint8_t spi_id);
static void send_uart(const char *path, char byte);
static int  get_vol_on_i2c(int chain);
static bool set_vol_on_i2c(int chain, int vol);
static bool set_power_on_i2c(int chain, int val);


pthread_mutex_t g_i2c_lock;


/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/

bool mcompat_i2c_feed_watchdog()
{
	/* Do something to keep i2c power alive */
	return get_vol_on_i2c(0) != -1;
}

bool hub_set_vid(uint8_t chain_id, int vol)
{
    switch(g_vid_type)
	{
		case MCOMPAT_LIB_VID_I2C_TYPE:
			return hub_set_vid_i2c(chain_id, vol);
		case MCOMPAT_LIB_VID_UART_TYPE:
			return hub_set_vid_uart(chain_id, vol);
		case MCOMPAT_LIB_VID_VID_TYPE:
		default:
			return hub_set_vid_vid(chain_id, vol);
	}
}

static bool hub_set_vid_vid(uint8_t chain_id, int vid)
{
    uint32_t reg_val = 0;
    int i = 0;
    uint8_t vid_binary[16] = {0};

    for(i = 0; i < 8; i ++)
    {
        vid_binary[i] = ((vid >> i) & 0x1) ? 7 : 1;
        vid_binary[8+i] = (vid_binary[i] == 1) ? 7 : 1;
    }

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG32_OFFSET, 25000);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG33_OFFSET, (vid_binary[3] << 28) | (vid_binary[2] << 24) | (vid_binary[1] << 20) | (vid_binary[0] << 16) | 0xff);
    reg_val = 0;
    for(i = 0; i < 8; i ++)
    {
        reg_val = (vid_binary[i+4] << (i*4)) | reg_val;
    }

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG34_OFFSET, reg_val);
    reg_val = 0;
    for(i = 0; i < 4; i ++)
    {
        reg_val = (vid_binary[i+12] << (i*4)) | reg_val;
    }

    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG35_OFFSET, reg_val);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG39_OFFSET, 80 | (4 << 16));
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG4_OFFSET, 0x1 << chain_id);
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG3_OFFSET, 0x1 );

    usleep(100000);

    return true;
}

#define IIC_VOL_MAX                    (1530)

/* i2c电源配置协议 */
static bool hub_set_vid_i2c(uint8_t chan_id, int vid)
{
    int vol = s_vid_map[vid][1];

    if (vol > IIC_VOL_MAX)
        vol = IIC_VOL_MAX;

    set_vol_on_i2c(chan_id + 1 , vol);
    
    return true;
}

static void hub_set_vid_uart_select(uint8_t spi_id)
{
    /* 选路 */
    Xil_Peripheral_Out32(MCOMPAT_PERIPHERAL_S00_AXI_SLV_REG4_OFFSET, (0x1 << 16) | (0x1 << spi_id));
}

static bool hub_set_vid_uart(uint8_t chain_id, int vid)
{
    char byte = 0;
    byte = (char)s_vid_map[vid][2]; 

    /* 选路 */
    hub_set_vid_uart_select(chain_id);

    /* 设置 */
    send_uart("/dev/ttyPS1", byte);

    return true;
}

void hub_set_power_en_i2c(uint8_t chain_id,int value)
{
    set_power_on_i2c(chain_id + 1, value);
}

bool mcompat_i2c_set_timeout(int time)
{
	bool ret = true;
	int i;
	int fd;
	int sum = 0;
	unsigned char buffer[10] = {0};
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	pthread_mutex_lock(&g_i2c_lock);

	fd = open(I2C_DEVICE_NAME, O_RDWR);
	if(fd < 0) {
		mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:open %s failled: %d.", __FILE__, __LINE__, I2C_DEVICE_NAME, errno);
		ret = false;
		goto out2;
	}

	buffer[0] = 0x00;
	buffer[1] = 0xab;
	buffer[2] = 0x00;
	buffer[3] = 0x70;
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	buffer[6] = (time/30);
	for (i = 2; i < 7; i++)
		sum = sum + buffer[i];
	buffer[7] = sum & 0xff;
	buffer[8] = 0xcd;

	messages[0].addr  = I2C_SLAVE_ADDR;
	messages[0].flags = I2C_M_IGNORE_NAK;
	messages[0].len   = sizeof(buffer);
	messages[0].buf   = buffer;

	packets.msgs      = messages;
	packets.nmsgs     = 1;

	if(ioctl(fd, I2C_RDWR, &packets) < 0) {
		mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:write iic failled: %d.", __FILE__, __LINE__, errno);
		hub_i2c_reset();
		ret = false;
		goto out1;
	}

out1:
	close(fd);
out2:
	pthread_mutex_unlock(&g_i2c_lock);
	return ret;
}

static bool set_power_on_i2c(int chain, int val)
{
    bool ret = true;
    int i;
    int fd;
    int sum;
    unsigned char buffer[10] = {0};
    unsigned char outbuf[] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    pthread_mutex_lock(&g_i2c_lock);

    fd = open(I2C_DEVICE_NAME, O_RDWR);
    if(fd < 0){
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:open %s failled: %d.\n", __FILE__, __LINE__, I2C_DEVICE_NAME, errno);
        ret = false;
		goto out2;
    }

    buffer[0] = 0x00;
    buffer[1] = 0xab;
    buffer[2] = 0x00;
    buffer[3] = 0x85;
    buffer[4] = 0x00;
    buffer[5] = 0x02;
    buffer[6] = chain;
    if(val != 0){
        buffer[7] = 0x01;
    }else{
        buffer[7] = 0x02;
    }

    for(i=2; i<8; i++){
    sum = sum + buffer[i];
    }
    buffer[8] = sum & 0xff;
    buffer[9] = 0xcd;

    messages[0].addr  = I2C_SLAVE_ADDR;
    messages[0].flags = I2C_M_IGNORE_NAK;
    messages[0].len   = sizeof(buffer);
    messages[0].buf   = buffer;

    packets.msgs      = messages;
    packets.nmsgs     = 1;

    if(ioctl(fd, I2C_RDWR, &packets) < 0) {
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:write iic failled: %d.\n", __FILE__, __LINE__, errno);
        hub_i2c_reset();
		ret = false;
		goto out1;
    }

out1:
	close(fd);
out2:
	pthread_mutex_unlock(&g_i2c_lock);
	return ret;
}

static int get_vol_on_i2c(int chain)
{
    bool ret = true;
    int i,j;
    int fd;
    int sum;
    unsigned char buffer[12] = {0};
    unsigned char reg[12] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    pthread_mutex_lock(&g_i2c_lock);

    fd = open(I2C_DEVICE_NAME, O_RDWR);
    if(fd < 0){
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:open %s failled: %d.\n", __FILE__, __LINE__, I2C_DEVICE_NAME, errno);
        ret = false;
		goto out2;
    }

    buffer[0] = 0x00;
    buffer[1] = 0xab;
    buffer[2] = 0x00;
    buffer[3] = 0x02;
    buffer[4] = 0x00;
    buffer[5] = 0x04;
    buffer[6] = chain;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;
    for(i=2; i<10; i++){
        sum = sum + buffer[i];
    }
    buffer[10] = sum & 0xff;
    buffer[11] = 0xcd;

    memset(reg,0,sizeof(reg));

    messages[0].addr  = I2C_SLAVE_ADDR;  
    messages[0].flags = 0; 
    messages[0].len   = sizeof(buffer);  
    messages[0].buf   = buffer;

    messages[1].addr  = I2C_SLAVE_ADDR;  
    messages[1].flags = I2C_M_RD;  
    messages[1].len   = sizeof(reg);  
    messages[1].buf   = reg;

    packets.msgs      = messages;  
    packets.nmsgs     = 2; 

    if(ioctl(fd, I2C_RDWR, &packets) < 0) {  
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:write iic failled: %d.\n", __FILE__, __LINE__, errno);
        hub_i2c_reset();
		ret = false;
		goto out1;
    }

out1:
	close(fd);
out2:
	pthread_mutex_unlock(&g_i2c_lock);
	return ret;
}

static bool set_vol_on_i2c(int chain, int vol)
{
    bool ret = true;
    int i;
    int fd;
    int sum = 0;
    unsigned char buffer[12] = {0};
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    pthread_mutex_lock(&g_i2c_lock);

    fd = open(I2C_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:open %s failled: %d.\n", __FILE__, __LINE__, I2C_DEVICE_NAME, errno);
        ret = false;
		goto out2;
    }
    
    buffer[0] = 0x00;
    buffer[1] = 0xab;
	buffer[2] = 0x00;
	buffer[3] = 0x83;
	buffer[4] = 0x00;
	buffer[5] = 0x04;
	buffer[6] = chain;
	buffer[7] = 0x00;
	buffer[8] = ((vol >> 0) & 0xff);
	buffer[9] = ((vol >> 8) & 0xff);

	for(i=2; i<10; i++){
		sum = sum + buffer[i];
	}
	buffer[10] = sum & 0xff;
	buffer[11] = 0xcd;
    
    messages[0].addr  = I2C_SLAVE_ADDR;  
    messages[0].flags = I2C_M_IGNORE_NAK;  
    messages[0].len   = sizeof(buffer);  
    messages[0].buf   = buffer;

    packets.msgs      = messages;  
    packets.nmsgs     = 1; 

    if(ioctl(fd, I2C_RDWR, &packets) < 0) {  
        mcompat_log(MCOMPAT_LOG_ERR, "%s,%d:write iic failled: %d.\n", __FILE__, __LINE__, errno);
        hub_i2c_reset();
		ret = false;
		goto out1;
    }     
    
out1:
    close(fd);
out2:
    pthread_mutex_unlock(&g_i2c_lock);
    return ret;
}

static void send_uart(const char *path, char byte)
{
    int tty_fd = 0;
    int rst = 0;
    char buf[5] = {0};

    tty_fd = open(path, O_WRONLY);
    if(-1 == tty_fd)
    {
        mcompat_log_err("%s,%d:open %s failled: %d.\n", __FILE__, __LINE__, path, errno);
    }
    /* mcompat_log(MCOMPAT_LOG_DEBUG, "%s,%d.\n", __FILE__, __LINE__); */

    buf[0] = 0xaa;
    buf[1] = 0x2;
    buf[2] = 0x1;
    buf[3] = byte;
    buf[4] = (~(buf[1]+buf[2]+buf[3]))+1;
    rst = write(tty_fd, &buf, 5);
    if(-1 == rst)
    {
        close(tty_fd);
        mcompat_log_err("%s,%d:write tty failled: %d.\n", __FILE__, __LINE__, errno);
    }

    /* for debug */
#if 0
    int i = 0;
    mcompat_log(MCOMPAT_LOG_DEBUG, "uart %s send:", path);
    for(i = 0; i < 5; i++)
    {
        mcompat_log_nt(MCOMPAT_LOG_DEBUG, "%02X,", buf[i]);
    }
    mcompat_log_nt(MCOMPAT_LOG_DEBUG, "\n");
#endif

    close(tty_fd);

    return;
}

