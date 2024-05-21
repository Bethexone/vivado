
/******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpiops.h"
//#include "xscugic.h"
//#include "xil_exception.h"
#include <xil_printf.h>



#define	XGPIOPS_BASEADDR	XPAR_XGPIOPS_0_BASEADDR

/************************** Variable Definitions *****************************/


/****************************************************************************/

int Gpio_init(XGpioPs *Gpio, UINTPTR BaseAddress)
{
	XGpioPs_Config *ConfigPtr;
	int Status;
	/* Initialize the Gpio driver. */
	ConfigPtr = XGpioPs_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XGpioPs_CfgInitialize(Gpio, ConfigPtr, ConfigPtr->BaseAddr);

	/* Run a self-test on the GPIO device. */
	Status = XGpioPs_SelfTest(Gpio);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
