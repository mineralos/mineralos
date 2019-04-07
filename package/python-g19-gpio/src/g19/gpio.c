#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <stdio.h>      
#include <stdint.h>      
#include <string.h>
#include <sys/types.h>      
#include <sys/stat.h>      
#include <sys/mman.h>


#define PAGE_SIZE  ((size_t)getpagesize())*2
#define PAGE_MASK ((uint32_t) (long)~(PAGE_SIZE - 1))

#define         LED_ON                  0
#define         LED_OFF                 1
#define         LED_BLING_ON            2
#define         LED_BLING_OFF           3

#define VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET 0
#define VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET 36
#define VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG10_OFFSET 40
#define XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR 0
volatile uint8_t *g_map_peripheral_base;
volatile uint8_t *g_map_base;

/**********************************************************
                Function:Xil_Mmap()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void Xil_Mmap(void)  
{
    int fd;
    uint32_t map_base = 0x43C00000;
    uint32_t peripheral_base = 0x43C10000;
    printf("xil_mmap in so...\n");
    if((fd = open("/dev/axi_fpga_dev", O_RDWR | O_SYNC)) == -1) 
    {  
        perror("open /dev/axi_fpga_dev:"); 
        return ;
    } 
 
    g_map_base = mmap(NULL, 0x3F000, PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0); 
    if(g_map_base == MAP_FAILED)  
    {
        perror("mmap:");
        printf("peripheral mmap failed! \n");
    }
    g_map_peripheral_base = g_map_base + peripheral_base - map_base;
    printf("xil_mmap in so over...\n");
    close(fd);
}

/**********************************************************
                Function:Xil_Out32()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void Xil_Out32(uint32_t phyaddr, uint32_t val)  
{   
    uint32_t pgoffset = phyaddr & (~PAGE_MASK); 
    //printf("reg_val1=0x%08x,On1=0x%08x\n",g_map_peripheral_base + pgoffset,val);
    *(volatile uint32_t *)(g_map_peripheral_base + pgoffset) = val; 
} 
 
/**********************************************************
                Function:Xil_In32()
                Date    :2018/1/3
                Version :1.0
*********************************************************/      
int Xil_In32(uint32_t phyaddr)  
{    
    uint32_t val;  
    uint32_t pgoffset = phyaddr & (~PAGE_MASK);
    val = *(volatile uint32_t *)(g_map_peripheral_base + pgoffset);
    return val;  
}

/**********************************************************
                Function:GPIOGreenLedOn()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void GPIOGreenLedOn(void)
{
    uint32_t reg_val,SetRedRegValue;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
    SetRedRegValue = reg_val | 0x200;
    //printf("SetRedRegValueon=%08x",SetRedRegValue);
    Xil_Out32(36,SetRedRegValue);
}

/**********************************************************
                Function:GPIOGreenLedOff()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void GPIOGreenLedOff(void)
{
    uint32_t reg_val,SetRedRegValue;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
    SetRedRegValue = reg_val & (~0x200);
    printf("SetRedRegValueoff=%08x",SetRedRegValue);
    Xil_Out32(36,SetRedRegValue);
}

/**********************************************************
                Function:GPIORedLedOn()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void GPIORedLedOn(void)
{
    uint32_t reg_val,SetRedRegValue;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
    //printf("reg_val_on=0x%08x\n",reg_val);
    SetRedRegValue = reg_val | 0x400;
    //printf("SetRedRegValue_on=0x%08x\n",SetRedRegValue);
    Xil_Out32(36,SetRedRegValue);
}

/**********************************************************
                Function:GPIORedLedOff()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
void GPIORedLedOff(void)
{
    uint32_t reg_val,SetRedRegValue;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
    //printf("reg_val_off=0x%08x\n",reg_val);
    SetRedRegValue = reg_val & (~0x400);
    //printf("SetRedRegValue_off=0x%08x\n",SetRedRegValue);
    Xil_Out32(36,SetRedRegValue);
}

/**********************************************************
                Function:GetKey_IPSet()
                Date    :2018/1/3
                Version :1.0
*********************************************************/
int GetKey_IPSet(void)
{
    uint32_t reg_val;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG10_OFFSET);
    return (((reg_val >> 16) & 0x1));
}

/**********************************************************
                Function:set_en_core()
                Date    :2018/1/3
                Version :1.0
*********************************************************/  
void set_en_core(uint32_t spi_id, uint32_t value)
{
    uint32_t reg_val;
    reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + spi_id*4);
    Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG0_OFFSET + spi_id*4, (reg_val & 0xfffeffff) | ((value & 0x1) << 16));
}

/**********************************************************
                Function:set_led()
                Date    :2018/1/3
                Version :1.0
*********************************************************/  
void set_led(uint32_t spi_id, uint32_t mode, uint32_t led_delay)
{
    uint32_t reg_val;

    if(mode == LED_ON){
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_ON & 0x1) << spi_id));
    }
    else if(mode == LED_OFF){
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_OFF & 0x1) << spi_id));
    }
    else if(mode == LED_BLING_ON){
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_OFF & 0x1) << spi_id));
        usleep(led_delay*1000);
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_ON & 0x1) << spi_id));
        usleep(led_delay*1000);
    }
    else if(mode == LED_BLING_OFF){
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_ON & 0x1) << spi_id));
        usleep(led_delay*1000);
        reg_val = Xil_In32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET);
        Xil_Out32(XPAR_VID_LED_BUZZER_CTRL_0_S00_AXI_BASEADDR + VID_LED_BUZZER_CTRL_S00_AXI_SLV_REG9_OFFSET,(reg_val & (0xffffffff ^ ( 1 << spi_id)) ) | ((LED_OFF & 0x1) << spi_id));
        usleep(led_delay*1000);
    }
}


