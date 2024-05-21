
#include "xaxivdma.h"
#include "xparameters.h"
#include "xil_printf.h"
#include <xil_types.h>
#include <xil_cache.h>


/* Memory space for the frame buffers
 *
 * This example only needs one set of frame buffers, because one video IP is
 * to write to the frame buffers, and the other video IP is to read from the
 * frame buffers.
 *
 * For 16 frames of 1080p, it needs 0x07E90000 memory for frame buffers
 */
#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_BASEADDRESS + 0x01000000)
#define MEM_HIGH_ADDR		XPAR_PS7_DDR_0_HIGHADDRESS
#define MEM_SPACE		(MEM_HIGH_ADDR - MEM_BASE_ADDR)
//0x100000 + 0x01000000=0x1100000
/* Read channel and write channel addresses
 */
#define READ_ADDRESS_BASE	MEM_BASE_ADDR
#define WRITE_ADDRESS_BASE	MEM_BASE_ADDR

/* Frame size related constants
 */
#define FRAME_HORIZONTAL_size 1920
#define BPP                    1                 //Bits Per Pixel
#define FRAME_HORIZONTAL_LEN  FRAME_HORIZONTAL_size * BPP   /* 1920 pixels, each pixel 4 bytes */
#define FRAME_VERTICAL_LEN    1080    /* 1080 pixels */

/* Subframe to be transferred by Video DMA
 *
 *|<----------------- FRAME_HORIZONTAL_LEN ---------------------->|
 * --------------------------------------------------------------------
 *|                                                                | ^
 *|                                                                | |
 *|               |<-SUBFRAME_HORIZONTAL_SIZE ->|                  | FRAME_
 *|               -----------------------------------              | VERTICAL_
 *|               |/////////////////////////////|  ^               | LEN
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////| SUBFRAME_        | |
 *|               |/////////////////////////////| VERTICAL_        | |
 *|               |/////////////////////////////| SIZE             | |
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////|  v               | |
 *|                ----------------------------------              | |
 *|                                                                | v
 *--------------------------------------------------------------------
 *
 * Note that SUBFRAME_HORIZONTAL_SIZE and SUBFRAME_VERTICAL_SIZE must ensure
 * to be inside the frame.
 */
#define SUBFRAME_START_OFFSET    (FRAME_HORIZONTAL_LEN * 5 + 64)
#define SUBFRAME_HORIZONTAL_SIZE 1280
#define SUBFRAME_VERTICAL_SIZE   720

/* Number of frames to transfer
 *
 * This is used to monitor the progress of the test, test purpose only
 */
// #define NUM_TEST_FRAME_SETS	10

// #define TEST_START_VALUE        0xC

/* Delay timer counter
 *
 * WARNING: If you are using fsync, please increase the delay counter value
 * to be 255. Because with fsync, the inter-frame delay is long. If you do not
 * care about inactivity of the hardware, set this counter to be 0, which
 * disables delay interrupt.
 */
#define DELAY_TIMER_COUNTER	0

/* Default reset timeout
 */
// #define XAXIVDMA_RESET_TIMEOUT_USEC	500
// #define POLL_TIMEOUT_COUNTER            1100000U
// #define NUMBER_OF_EVENTS		1
/*
 * Device instance definitions
 */

int BOTH = 3;
XAxiVdma AxiVdma;

//static XScuGic Intc;	/* Instance of the Interrupt Controller */

/* Data address
 *
 * Read and write sub-frame use the same settings
 */
static UINTPTR ReadFrameAddr;
static UINTPTR WriteFrameAddr;
static UINTPTR BlockStartOffset;
static UINTPTR BlockHoriz;
static UINTPTR BlockVert;

/* Frame-buffer count i.e Number of frames to work on
 */
volatile static u16 ReadCount;
volatile static u16 WriteCount;

/* DMA channel setup
 */
static XAxiVdma_DmaSetup ReadCfg;
static XAxiVdma_DmaSetup WriteCfg;


/******************* Function Prototypes ************************************/

int ReadSetup(XAxiVdma *InstancePtr);
int WriteSetup(XAxiVdma *InstancePtr);
int StartTransfer(XAxiVdma *InstancePtr,u16 vdma_mode);
int CheckFrame(int FrameIndex);

int VDMA_Setup(UINTPTR VDMA_BASEADDR,XAxiVdma *Vdma_InstancePtr,u16 vdma_mode)
{
	int Status;
	XAxiVdma_Config *Config;
	XAxiVdma_FrameCounter FrameCfg;

	ReadFrameAddr = READ_ADDRESS_BASE;
	WriteFrameAddr = WRITE_ADDRESS_BASE;
	BlockStartOffset = SUBFRAME_START_OFFSET;
	BlockHoriz = SUBFRAME_HORIZONTAL_SIZE;
	BlockVert = SUBFRAME_VERTICAL_SIZE;

	xil_printf("\r\n--- Entering VDMA_Setup --- \r\n");

	/* The information of the XAxiVdma_Config comes from hardware build.
	 * The user IP should pass this information to the AXI DMA core.
	 */
	Config = XAxiVdma_LookupConfig(VDMA_BASEADDR);
	if (!Config) {
		xil_printf(
			"No video DMA found for Address %llx\r\n", XPAR_XAXIVDMA_0_BASEADDR);

		return XST_FAILURE;
	}


	/* Set default read and write count based on HW config*/
	ReadCount = Config->MaxFrameStoreNum;
	WriteCount = Config->MaxFrameStoreNum;

    memset((void*)WRITE_ADDRESS_BASE, 0x00, FRAME_HORIZONTAL_LEN*FRAME_VERTICAL_LEN*WriteCount); 
	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(Vdma_InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {

		xil_printf(
			"Configuration Initialization failed %d\r\n", Status);

		return XST_FAILURE;
	}
    

	/* Setup frame counter and delay counter for both channels
	 *
	 * This is to monitor the progress of the test only
	 *
	 * WARNING: In free-run mode, interrupts may overwhelm the system.
	 * In that case, it is better to disable interrupts.
	 */
	FrameCfg.ReadFrameCount = ReadCount;
	FrameCfg.WriteFrameCount = WriteCount;
	FrameCfg.ReadDelayTimerCount = DELAY_TIMER_COUNTER;
	FrameCfg.WriteDelayTimerCount = DELAY_TIMER_COUNTER;

	Status = XAxiVdma_SetFrameCounter(Vdma_InstancePtr, &FrameCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Set frame counter failed %d\r\n", Status);

		if (Status == XST_VDMA_MISMATCH_ERROR) {
			xil_printf("DMA Mismatch Error\r\n");
		}

		return XST_FAILURE;
	}

	/*
	 * Setup your video IP that writes to the memory
	 */


	/* Setup the write channel
	 */
    if ((vdma_mode == XAXIVDMA_WRITE)||(vdma_mode == BOTH)){     
	    Status = WriteSetup(Vdma_InstancePtr);
	    if (Status != XST_SUCCESS) {
	    	xil_printf(
	    		"Write channel setup failed %d\r\n", Status);
	    	if (Status == XST_VDMA_MISMATCH_ERROR) {
	    		xil_printf("DMA Mismatch Error\r\n");
	    	}
    
	    	return XST_FAILURE;
	    }
    }

	/*
	 * Setup your video IP that reads from the memory
	 */

    /* Setup the read channel
     */
    if ((vdma_mode == XAXIVDMA_READ)||(vdma_mode == BOTH)){
	    Status = ReadSetup(Vdma_InstancePtr);
	    if (Status != XST_SUCCESS) {
	    	xil_printf(
	    		"Read channel setup failed %d\r\n", Status);
	    	if (Status == XST_VDMA_MISMATCH_ERROR) {
	    		xil_printf("DMA Mismatch Error\r\n");
            }
        }   
    }
    Status = StartTransfer(Vdma_InstancePtr, vdma_mode);

    return Status;
       
}
/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	InstancePtr is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
***************************************************************************/
int ReadSetup(XAxiVdma *InstancePtr)
{
	int Index;
	UINTPTR Addr;
	int Status;

	ReadCfg.VertSizeInput = SUBFRAME_VERTICAL_SIZE;
	ReadCfg.HoriSizeInput = SUBFRAME_HORIZONTAL_SIZE;

	ReadCfg.Stride = FRAME_HORIZONTAL_LEN;
	ReadCfg.FrameDelay = 1;  /* This example  test frame delay */

	ReadCfg.EnableCircularBuf = 1;
	ReadCfg.EnableSync = 1;  /* enable Gen-Lock */

	ReadCfg.PointNum = 0;    /* No Gen-Lock */
	ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

	ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

	Status = XAxiVdma_DmaConfig(InstancePtr, XAXIVDMA_READ, &ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Read channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = READ_ADDRESS_BASE + BlockStartOffset;
	for (Index = 0; Index < ReadCount; Index++) {
		ReadCfg.FrameStoreStartAddr[Index] = Addr;

		Xil_DCacheFlushRange(Addr, FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN);
		Addr += FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(InstancePtr, XAXIVDMA_READ,
					   ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Read channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	InstancePtr is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int WriteSetup(XAxiVdma *InstancePtr)
{
	int Index;
	UINTPTR Addr;
	int Status;

	WriteCfg.VertSizeInput = SUBFRAME_VERTICAL_SIZE;
	WriteCfg.HoriSizeInput = SUBFRAME_HORIZONTAL_SIZE;

	WriteCfg.Stride = FRAME_HORIZONTAL_LEN;
	WriteCfg.FrameDelay = 0;  /* This example does not test frame delay */

	WriteCfg.EnableCircularBuf = 1;
	WriteCfg.EnableSync = 0;  /* No Gen-Lock */

	WriteCfg.PointNum = 0;    /* No Gen-Lock */
	WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

	WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

	WriteCfg.EnableVFlip = 0; /* Enable vertical flip */

	Status = XAxiVdma_DmaConfig(InstancePtr, XAXIVDMA_WRITE, &WriteCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Write channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * Use physical addresses
	 */
    Addr = WRITE_ADDRESS_BASE + BlockStartOffset;   
	for (Index = 0; Index < WriteCount; Index++) {
		WriteCfg.FrameStoreStartAddr[Index] = Addr;

		Addr += FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 */
	Status = XAxiVdma_DmaSetBufferAddr(InstancePtr, XAXIVDMA_WRITE,
					   WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Write channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Clear data buffer
	 */
	memset((void *)WriteFrameAddr, 0,
	       FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN * WriteCount);
	Xil_DCacheFlushRange(WriteFrameAddr, FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN * WriteCount);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function starts the DMA transfers. Since the DMA engine is operating
* in circular buffer mode, video frames will be transferred continuously.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return	XST_SUCCESS if both read and write start successfully
*		XST_FAILURE if one or both directions cannot be started
*
* @note		None.
*
***********************************************************************/
int StartTransfer(XAxiVdma *InstancePtr,u16 vdma_mode)
{   
	int Status;
    if ((vdma_mode == XAXIVDMA_WRITE)||(vdma_mode == BOTH)){
	    Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_WRITE);
	    if (Status != XST_SUCCESS) {
              xil_printf("Start Write transfer failed %d\r\n", Status);
            
		    return XST_FAILURE;
        }
    }
    if ((vdma_mode == XAXIVDMA_READ)||(vdma_mode == BOTH)){
	    Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
	    if (Status != XST_SUCCESS) {
	    	xil_printf(
	    		"Start read transfer failed %d\r\n", Status);

	    	return XST_FAILURE;
	    }
    }
    return XST_SUCCESS;
}
