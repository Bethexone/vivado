

#include "xiicps.h"
#include "iic_init.h"

int iic_init(XIicPs* Iic,UINTPTR BaseAddress,u32 IIC_SCLK_RATE)
{ 
	int Status;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */

	Config = XIicPs_LookupConfig(BaseAddress);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
  
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(Iic, IIC_SCLK_RATE);
    while (XIicPs_BusIsBusy(Iic));	// Wait    
	return XST_SUCCESS;
}

int i2c_reg8_write(XIicPs *InstancePtr, char IIC_ADDR, char Addr, char Data)
{
	int Status;
	u8 SendBuffer[2];

	SendBuffer[0] = Addr;
	SendBuffer[1] = Data;
	Status = XIicPs_MasterSendPolled(InstancePtr, SendBuffer, 2, IIC_ADDR);
	while (XIicPs_BusIsBusy(InstancePtr));

	return Status;
}

char i2c_reg8_read(XIicPs *InstancePtr, char IIC_ADDR, char Addr)
{
	u32 Status;
	u8 wr_data, rd_data;

	wr_data = Addr;
	// Set the IIC Repeated Start option.
	Status = XIicPs_SetOptions(InstancePtr, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIicPs_MasterSendPolled(InstancePtr, &wr_data, 1, IIC_ADDR);
	XIicPs_MasterRecvPolled(InstancePtr, &rd_data, 1, IIC_ADDR);
	while (XIicPs_BusIsBusy(InstancePtr));

	// Clear the IIC Repeated Start option.
	Status = XIicPs_ClearOptions(InstancePtr, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return rd_data;
}

int i2c_reg16_write(XIicPs *InstancePtr, char IIC_ADDR, unsigned short Addr, char Data)
{
	int Status;
	u8 SendBuffer[3];

	SendBuffer[0] = Addr>>8;
	SendBuffer[1] = Addr;
	SendBuffer[2] = Data;
	Status = XIicPs_MasterSendPolled(InstancePtr, SendBuffer, 3, IIC_ADDR);
	while (XIicPs_BusIsBusy(InstancePtr));

	return Status;
}

char i2c_reg16_read(XIicPs *InstancePtr, char IIC_ADDR, unsigned short Addr)
{
	u8 rd_data;
	u8 SendBuffer[2];

	SendBuffer[0] = Addr>>8;
	SendBuffer[1] = Addr;
	XIicPs_MasterSendPolled(InstancePtr, SendBuffer, 2, IIC_ADDR);
	XIicPs_MasterRecvPolled(InstancePtr, &rd_data, 1, IIC_ADDR);
	while (XIicPs_BusIsBusy(InstancePtr));
	return rd_data;
}