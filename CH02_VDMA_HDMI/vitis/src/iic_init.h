#include "xiicps.h"
//#include "xil_printf.h"

int iic_init(XIicPs *Iic, UINTPTR BaseAddress, u32 IIC_SCLK_RATE);
int i2c_reg8_write(XIicPs *InstancePtr, char IIC_ADDR, char Addr, char Data);
char i2c_reg8_read(XIicPs *InstancePtr, char IIC_ADDR, char Addr);
int i2c_reg16_write(XIicPs *InstancePtr, char IIC_ADDR, unsigned short Addr, char Data);
char i2c_reg16_read(XIicPs *InstancePtr, char IIC_ADDR, unsigned short Addr);