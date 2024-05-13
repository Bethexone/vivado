
/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */


#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
//#include "xil_io.h"
#include "iic_init.h"
#include "xaxivdma.h"
//#include "xil_cache.h"
#include <xil_types.h>


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

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

XIicPs ps_i2c0;

XAxiVdma video;

void LT8618_init(XIicPs IicInstance) ;
void LT8618SX_Initial(XIicPs *InstancePtr, char IIC_ADDR);
int VDMA_Setup(UINTPTR VDMA_BASEADDR, XAxiVdma *Vdma_InstancePtr,u16 vdma_mode);
int main(void)
{
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

     //库函数初始化VDMA
    VDMA_Setup(VDMA0_BASEADDR, &video, 3);
    
    xil_printf("IIC init \r\n");

    iic_init(&ps_i2c0, XPAR_XIICPS_0_BASEADDR, 100000);
    
    xil_printf("LT8618SX init \r\n");
        
    LT8618SX_Initial(&ps_i2c0, LT8618SX_ADR >> 1 );
	while (1) ;
	return XST_SUCCESS;
}

