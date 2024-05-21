
/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

//#include <cstdint>
#include <stdio.h>
#include "sleep.h"
#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
//#include "xil_io.h"
#include "iic_init.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include <xil_types.h>
#include "ov5640.h"
#include "xgpiops.h"
#include "xil_types.h"
/*
 * XPAR redefines
 */
#define HORSIZE 1280
#define VERSIZE 720
#define BPP     1   //Bits Per Pixel
#define DEMO_MAX_FRAME HORSIZE * VERSIZE * BPP
#define VDMA0_BASEADDR XPAR_AXI_VDMA_0_BASEADDR

//#define   LT8618SX_ADR		0x76                    // IIC Address If CI2CA pins are high(2800mV~3300mV)
#define   LT8618SX_ADR 0x72                     // IIC Address If CI2CA pins are low(0~400mV)

#define GPIO_BASEADDR XPAR_XGPIOPS_0_BASEADDR
/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

XIicPs ps_i2c0;
XIicPs ps_i2c1;

XAxiVdma video;

XGpioPs cam_gpio;

int Gpio_init(XGpioPs *Gpio, UINTPTR BaseAddress);
void LT8618_init(XIicPs IicInstance) ;
void LT8618SX_Initial(XIicPs *InstancePtr, char IIC_ADDR);
int VDMA_Setup(UINTPTR VDMA_BASEADDR, XAxiVdma *Vdma_InstancePtr,u16 vdma_mode);
void enable_caches();

static u32 cam_rst = 54; 
static u32 cam_PWDN = 55;



int main(void) {
  
    enable_caches();    
    u32 Status;
  
    //Xil_DCacheDisable();
    //Xil_ICacheDisable();

    //寄存器初始化VDMA
    // Xil_Out32((VDMA_BASEADDR + 0x030), 0x108B);// enable circular mode
    // Xil_Out32((VDMA_BASEADDR + 0x0AC), VIADO1_IN_BUF_ADDR[0]);	// start
    // address Xil_Out32((VDMA_BASEADDR + 0x0B0), VIADO1_IN_BUF_ADDR[1]);
    // // start address Xil_Out32((VDMA_BASEADDR + 0x0B4),
    // VIADO1_IN_BUF_ADDR[2]);	// start address Xil_Out32((VDMA_BASEADDR +
    // 0x0A8), (1920*1));		// h offset 1280 bytes
    // Xil_Out32((VDMA_BASEADDR + 0x0A4), (HORSIZE*1));		// h size 1280
    // bytes Xil_Out32((VDMA_BASEADDR + 0x0A0), VERSIZE);
    // // v size 1024
    // 	/*****************DDR**********************/
    // Xil_Out32((VDMA_BASEADDR + 0x000), 0x8B); 		// enable
    // circular mode Xil_Out32((VDMA_BASEADDR + 0x05c), VIADO_OUT_BUF_ADDR[0]);
    // // start address Xil_Out32((VDMA_BASEADDR + 0x060),
    // VIADO_OUT_BUF_ADDR[1]); 	// start address Xil_Out32((VDMA_BASEADDR +
    // 0x064), VIADO_OUT_BUF_ADDR[2]); 	// start address
    // Xil_Out32((VDMA_BASEADDR + 0x058), (1920*1)); 		// h offset 1280
    // bytes Xil_Out32((VDMA_BASEADDR + 0x054), (HORSIZE*1)); 		// h
    // size 1280 bytes Xil_Out32((VDMA_BASEADDR + 0x050), VERSIZE);
    // // v size 1024

   
    // 库函数初始化VDMA
    xil_printf("VDMA init \r\n");
    Status = VDMA_Setup(VDMA0_BASEADDR, &video, 3);
    if (Status != XST_SUCCESS) {
		xil_printf(
			"VDMA_Setup Initialization failed %d\r\n", Status);
    }
    else xil_printf(
			"VDMA_Setup SUCCESS %d\r\n", Status);


    
    xil_printf("ps_i2c0 init \r\n");
    Status = iic_init(&ps_i2c0, XPAR_XIICPS_0_BASEADDR, 100000);
        if (Status != XST_SUCCESS) {
	    	xil_printf(
	    		"ps_i2c0 Initialization failed %d\r\n", Status);
        }
        else xil_printf(
	    		"ps_i2c0 SUCCESS %d\r\n", Status);


    
    xil_printf("ps_i2c1 init \r\n");
    Status = iic_init(&ps_i2c1, XPAR_XIICPS_1_BASEADDR, 100000);
        if (Status != XST_SUCCESS) {

	    	xil_printf(
	    		"ps_i2c1 Initialization failed %d\r\n", Status);
        }
        else xil_printf(
			"ps_i2c1 SUCCESS %d\r\n", Status);


    xil_printf("LT8618SX init \r\n");
    LT8618SX_Initial(&ps_i2c1, LT8618SX_ADR>>1);
    sleep(1);

    xil_printf("cam_gpio init \r\n");
    Gpio_init(&cam_gpio, GPIO_BASEADDR);
    if (Status != XST_SUCCESS) {

	    	xil_printf(
	    		"cam_gpio Initialization failed %d\r\n", Status);
        }
    else
          xil_printf("cam_gpio SUCCESS %d\r\n", Status);

    /* Set the direction for the specified pin to be output. */
    XGpioPs_SetDirectionPin(&cam_gpio, cam_rst, 0x1);
    XGpioPs_SetDirectionPin(&cam_gpio, cam_PWDN, 0x1);
	/* enable for the specified pin to be output. */
    XGpioPs_SetOutputEnablePin(&cam_gpio, cam_rst, 1);
    XGpioPs_SetOutputEnablePin(&cam_gpio, cam_PWDN, 1);


    XGpioPs_WritePin(&cam_gpio, cam_rst, 0x0);
    XGpioPs_WritePin(&cam_gpio, cam_PWDN, 0x1);
    usleep(10000);
    XGpioPs_WritePin(&cam_gpio, cam_PWDN, 0x0);
    XGpioPs_WritePin(&cam_gpio, cam_rst, 0x1);    

    usleep(1000000);

    sensor_init(&ps_i2c0);
    sleep(1);

	return XST_SUCCESS;
}

void enable_caches()
{
    #ifdef __PPC__
    Xil_ICacheEnableRegion(CACHEABLE_REGION_MASK);
    Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
    #elif __MICROBLAZE__
    #ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
    #endif
    #ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
    #endif
    #endif
}