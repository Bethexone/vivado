#include "ov5640.h"
#include "xiicps.h"
//#include "xparameters.h"
#include "iic_init.h"
#include "sleep.h"
#include <xil_types.h>
// #include "config.h"

int XVCLK = 2400;//real clk *10000

u32 Status;

struct reginfo sensor_init_data[] =
{	
		//[7]=0 Software reset; [6]=1 Software power down; Default=0x02
		{0x3008, 0x42},
		//[1]=1 System input clock from PLL; Default read = 0x11
		{0x3103, 0x03},
		//[3:0]=0000 MD2P,MD2N,MCP,MCN input; Default=0x00
		{0x3017, 0x00},
		//[7:2]=000000 MD1P,MD1N, D3:0 input; Default=0x00
		{0x3018, 0x00},
		//[6:4]=001 PLL charge pump, [3:0]=1000 MIPI 8-bit mode
		{0x3034, 0x18},
		//PLL1 configuration
		//[7:4]=0001 System clock divider /1, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x11},
		//[7:0]=56 PLL multiplier
		{0x3036, 0x38},
		//[4]=1 PLL root divider /2, [3:0]=1 PLL pre-divider /1
		{0x3037, 0x11},
		//[5:4]=00 PCLK root divider /1, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x01},
		//PLL2 configuration
		//[5:4]=01 PRE_DIV_SP /1.5, [2]=1 R_DIV_SP /1, [1:0]=00 DIV12_SP /1
		{0x303D, 0x10},
		//[4:0]=11001 PLL2 multiplier DIV_CNT5B = 25
		{0x303B, 0x19},

		{0x3630, 0x2e},
		{0x3631, 0x0e},
		{0x3632, 0xe2},
		{0x3633, 0x23},
		{0x3621, 0xe0},
		{0x3704, 0xa0},
		{0x3703, 0x5a},
		{0x3715, 0x78},
		{0x3717, 0x01},
		{0x370b, 0x60},
		{0x3705, 0x1a},
		{0x3905, 0x02},
		{0x3906, 0x10},
		{0x3901, 0x0a},
		{0x3731, 0x02},
		//VCM debug mode
		{0x3600, 0x37},
		{0x3601, 0x33},
		//System control register changing not recommended
		{0x302d, 0x60},
		//??
		{0x3620, 0x52},
		{0x371b, 0x20},
		//?? DVP
		{0x471c, 0x50},

		{0x3a13, 0x43},
		{0x3a18, 0x00},
		{0x3a19, 0xf8},
		{0x3635, 0x13},
		{0x3636, 0x06},
		{0x3634, 0x44},
		{0x3622, 0x01},
		{0x3c01, 0x34},
		{0x3c04, 0x28},
		{0x3c05, 0x98},
		{0x3c06, 0x00},
		{0x3c07, 0x08},
		{0x3c08, 0x00},
		{0x3c09, 0x1c},
		{0x3c0a, 0x9c},
		{0x3c0b, 0x40},

		//[7]=1 color bar enable, [3:2]=00 eight color bar
		{0x503d, 0x00},
        //{0x503d,0x80},
        //{0x4741,0x01},
		//[2]=1 ISP vflip, [1]=1 sensor vflip
		{0x3820, 0x46},

		//[7:5]=001 Two lane mode, [4]=0 MIPI HS TX no power down, [3]=0 MIPI LP RX no power down, [2]=1 MIPI enable, [1:0]=10 Debug mode; Default=0x58
		{0x300e, 0x45},
		//[5]=0 Clock free running, [4]=1 Send line short packet, [3]=0 Use lane1 as default, [2]=1 MIPI bus LP11 when no packet; Default=0x04
		{0x4800, 0x14},
		{0x302e, 0x08},
		//[7:4]=0x3 YUV422, [3:0]=0x0 YUYV
		//{0x4300, 0x30},
		//[7:4]=0x6 RGB565, [3:0]=0x0 {b[4:0],g[5:3],g[2:0],r[4:0]}
		{0x4300, 0x6f},
		{0x501f, 0x01},

		{0x4713, 0x03},
		{0x4407, 0x04},
		{0x440e, 0x00},
		{0x460b, 0x35},
		//[1]=0 DVP PCLK divider manual control by 0x3824[4:0]
		{0x460c, 0x20},
		//[4:0]=1 SCALE_DIV=INT(3824[4:0]/2)
		{0x3824, 0x01},
		//[7]=1 LENC correction enabled, [5]=1 RAW gamma enabled, [2]=1 Black pixel cancellation enabled, [1]=1 White pixel cancellation enabled, [0]=1 Color interpolation enabled
		{0x5000, 0x07},
		//[7]=0 Special digital effects, [5]=0 scaling, [2]=0 UV average disabled, [1]=1 Color matrix enabled, [0]=1 Auto white balance enabled
		{0x5001, 0x03},
        {0x4202, 0x0f},
		{SEQUENCE_END, 0x00}
};


struct reginfo cfg_720p_60fps[] =
{
		//1280 x 720 binned, RAW10, MIPISCLK=280M, SCLK=56Mz, PCLK=56M
		//PLL1 configuration
		//[7:4]=0010 System clock divider /2, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x21},
		//[7:0]=70 PLL multiplier
		{0x3036, 0x46},
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		//[5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x1A},

		//[3:0]=0 X address start high byte
		{0x3800, (0 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801, 0 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (8 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 8 & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (2619 >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 2619 & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (1947 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 1947 & 0xFF},

		//[3:0]=0 timing hoffset high byte
		{0x3810, (0 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 0 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (0 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 0 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1280 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1280 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (720 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 720 & 0xFF},

		//HTS line exposure time in # of pixels
		{0x380c, (1896 >> 8) & 0x1F},
		{0x380d, 1896 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (984 >> 8) & 0xFF},
		{0x380f, 984 & 0xFF},

		//[7:4]=0x3 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x31},
		//[7:4]=0x3 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x31},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=1 horizontal binning
		{0x3821, 0x01},

		//little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 36}, // 1/56M*2

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03},
		{SEQUENCE_END, 0x00}
};

struct reginfo cfg_720p_60fps_raw8[] = {


		//1280 x 720 binned, RAW8, MIPISCLK=280M, SCLK=56Mz, PCLK=70M
		//PLL1 configuration
		//[7:4]=0010 System clock divider /2, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x21},
		//[7:0]=70 PLL multiplier
		{0x3036, 0x46},
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		//[5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1000 MIPI 8-bit mode
		{0x3034, 0x18},

		//[3:0]=0 X address start high byte
		{0x3800, (0 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801,0 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (0 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 0xfa & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (0x0a3f >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 0x0a3f & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (0x06a9 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 0x06a9 & 0xFF},
        
		//[3:0]=0 timing hoffset high byte
		{0x3810, (0 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 0x10 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (0 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 0x04 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1280 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1280 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (720 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 720 & 0xFF},

		//HTS line exposure time in # of pixels
		{0x380c, (0x764 >> 8) & 0x1F},
		{0x380d, 0x764 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (0x2e4 >> 8) & 0xFF},
		{0x380f, 0x2e4 & 0xFF},
        
        
		//[7:4]=0x3 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x31},
		//[7:4]=0x3 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x31},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=1 horizontal binning
		{0x3821, 0x01},

		//little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 14}, // 1/84M*2

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03},
		{SEQUENCE_END, 0x00}
        
        // //[3:0]=0 X address start high byte
		// {0x3800, (0 >> 8) & 0x0F},
		// //[7:0]=0 X address start low byte
		// {0x3801, 0 & 0xFF},
		// //[2:0]=0 Y address start high byte
		// {0x3802, (8 >> 8) & 0x07},
		// //[7:0]=0 Y address start low byte
		// {0x3803, 8 & 0xFF},

		// //[3:0] X address end high byte
		// {0x3804, (2619 >> 8) & 0x0F},
		// //[7:0] X address end low byte
		// {0x3805, 2619 & 0xFF},
		// //[2:0] Y address end high byte
		// {0x3806, (1947 >> 8) & 0x07},
		// //[7:0] Y address end low byte
		// {0x3807, 1947 & 0xFF},
        // //HTS line exposure time in # of pixels
		// {0x380c, (1896 >> 8) & 0x1F},
		// {0x380d, 1896 & 0xFF},
		// //VTS frame exposure time in # lines
		// {0x380e, (984 >> 8) & 0xFF},
		// {0x380f, 984 & 0xFF},
};

struct reginfo cfg_1080p_30fps[] =
{       //1920 x 1080 @ 30fps, RAW10, MIPISCLK=420, SCLK=84MHz, PCLK=84M
		//PLL1 configuration
		//[7:4]=0010 System clock divider /2, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x21}, // 30fps setting
		//[7:0]=105 PLL multiplier
		{0x3036, 0x69},
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		//[5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x1A},

		//[3:0]=0 X address start high byte
		{0x3800, (336 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801, 336 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (426 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 426 & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (2287 >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 2287 & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (1529 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 1529 & 0xFF},

		//[3:0]=0 timing hoffset high byte
		{0x3810, (16 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 16 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (12 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 12 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1920 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1920 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (1080 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 1080 & 0xFF},

		//HTS line exposure time in # of pixels Tline=HTS/sclk
		{0x380c, (2500 >> 8) & 0x1F},
		{0x380d, 2500 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (1120 >> 8) & 0xFF},
		{0x380f, 1120 & 0xFF},

		//[7:4]=0x1 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x11},
		//[7:4]=0x1 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x11},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=0 no horizontal binning
		{0x3821, 0x00},

		//little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 24}, // 1/84M*2

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03},

		{SEQUENCE_END, 0x00}
};

struct reginfo cfg_1080p_30fps_raw8[] =
{       //1920 x 1080 @ 30fps, RAW10, MIPISCLK=420, SCLK=84MHz, PCLK=84M
		//PLL1 configuration
		//[7:4]=0010 System clock divider /2, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x21}, // 30fps setting
		//[7:0]=105 PLL multiplier
		{0x3036, 0x69},
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		//[5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x18},

		//[3:0]=0 X address start high byte
		{0x3800, (336 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801, 336 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (426 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 426 & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (2287 >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 2287 & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (1529 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 1529 & 0xFF},

		//[3:0]=0 timing hoffset high byte
		{0x3810, (16 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 16 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (12 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 12 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1920 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1920 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (1080 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 1080 & 0xFF},

		//HTS line exposure time in # of pixels Tline=HTS/sclk
		{0x380c, (2500 >> 8) & 0x1F},
		{0x380d, 2500 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (1120 >> 8) & 0xFF},
		{0x380f, 1120 & 0xFF},

		//[7:4]=0x1 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x11},
		//[7:4]=0x1 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x11},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=0 no horizontal binning
		{0x3821, 0x00},

		//little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 10}, // 1/84M*2

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03},

		{SEQUENCE_END, 0x00}
};

struct reginfo cfg_advanced_awb[] =
{
		// Enable Advanced AWB
		{0x3406 ,0x00},
		{0x5192 ,0x04},
		{0x5191 ,0xf8},
		{0x518d ,0x26},
		{0x518f ,0x42},
		{0x518e ,0x2b},
		{0x5190 ,0x42},
		{0x518b ,0xd0},
		{0x518c ,0xbd},
		{0x5187 ,0x18},
		{0x5188 ,0x18},
		{0x5189 ,0x56},
		{0x518a ,0x5c},
		{0x5186 ,0x1c},
		{0x5181 ,0x50},
		{0x5184 ,0x20},
		{0x5182 ,0x11},
		{0x5183 ,0x00},
		{0x5001 ,0x03},

		{SEQUENCE_END, 0x00}
};

int ov5640_read(XIicPs *IicInstance,u16 addr,u8 *read_buf)
{
  *read_buf=i2c_reg16_read(IicInstance,0x3c,addr);
	return XST_SUCCESS;
}

int ov5640_write(XIicPs *IicInstance,u16 addr,u8 data)
{
    
	return i2c_reg16_write(IicInstance,0x3c,addr,data);
}

/* write a array of registers  */
void sensor_write_array(XIicPs *IicInstance, struct reginfo *regarray)
{
    int i = 0;
	while (regarray[i].reg != SEQUENCE_END) {
        Status=ov5640_write(IicInstance, regarray[i].reg, regarray[i].val);
        // if (Status != XST_SUCCESS) {
	    // 	xil_printf(
	    // 		"ov5640_write failed 0x%x\r\n", regarray[i].reg);
        // }
        // else xil_printf(
	    // 		"ov5640_write SUCCESS 0x%x\r\n", regarray[i].reg);

		i++;
	}

}

int ov5640_auto_focus(XIicPs *IicInstance) {
  u8 temp;
  // focus
  ov5640_write(IicInstance, 0x3022, 0x03);
  while (1) {
    // check status
    ov5640_read(IicInstance, 0x3029, &temp);
    if (temp == 0x10) {
      ov5640_write(IicInstance, 0x3022, 0x06);
      return 0; // focus completed
      
    }
      return 0; // focus completed
    return 1;

    usleep(100);
  }

}
int ov5640_get_sysclk(XIicPs *IicInstance) {

    // calculate sysclk
    u8 temp1, temp2;
    int Multiplier, VCO, SysDiv,Mipi_div, Pll_rdiv, sclk_rdiv, pclk_div,p_div,sysclk,MIPI_clk,P_clk;
    float PreDiv,Bit_div;
    float PreDiv_map[]={1,1,2,3,4,1.5,6,2.5,8,1,1,1,1,1,1,1};
    int sclk_rdiv_map[] = {1, 2, 4, 8};

    ov5640_read(IicInstance,0x3034, &temp1);
    temp2 = temp1 & 0x0f;
    if (temp2 == 8 || temp2 == 10) {
      Bit_div=temp2/(4.0);
    }
    else
      Bit_div=1;
    
    ov5640_read(IicInstance, 0x3035, &temp1);
    SysDiv = temp1 >> 4;
    if (SysDiv == 0) {
      SysDiv = 16;
    }
    Mipi_div = temp1 & 0x0f;
    if (Mipi_div == 0) {
      Mipi_div = 16;
    }
    // mipi_2lane
    p_div = Mipi_div;

    ov5640_read(IicInstance, 0x3037, &temp1);
    temp2 =temp1>>7;
    PreDiv = PreDiv_map[temp1 & 0x0f];
    Pll_rdiv = ((temp1 >> 4) & 0x01) + 1;

    ov5640_read(IicInstance, 0x3036, &temp1);
    if (temp2)
      Multiplier = (temp1 >> 1) * 2;
    else Multiplier = temp1&0x7f;

    ov5640_read(IicInstance, 0x3108, &temp1);
    temp2 = temp1 & 0x03;
    sclk_rdiv = sclk_rdiv_map[temp2];
    temp2 = temp1 & 0x30;
    pclk_div = sclk_rdiv_map[temp2>>4];    

    VCO = XVCLK * Multiplier / PreDiv;
    sysclk = VCO / SysDiv / Pll_rdiv  / Bit_div / sclk_rdiv;

    MIPI_clk = VCO / SysDiv / Mipi_div / 2;

    P_clk = VCO / SysDiv / Pll_rdiv  / Bit_div / pclk_div / p_div/2;
    
    xil_printf("sysclk:%d  MIPI_clk:%d P_clk:%d\n", sysclk/2,MIPI_clk,P_clk);
    return sysclk;
   
    }

int ov5640_get_HTS(XIicPs *IicInstance) {
    // read HTS from register settings
    u8 temp1, temp2;
    u16 HTS;
    ov5640_read(IicInstance, 0x380c, &temp1);
    ov5640_read(IicInstance, 0x380d, &temp2);
    //xil_printf("0x380c:%x 0x380d: %x", temp1, temp2);
    HTS = temp1;
    HTS = (HTS << 8) + temp2;

    // int i ;
    // for (i = 0; i < 32; i++) {
    //     ov5640_read(IicInstance, 0x3800+i, &temp1);
    //     xil_printf("%x:%x ", 0x3800+i,temp1);
      
    //   }
    
    return HTS;
}

int ov5640_get_VTS(XIicPs *IicInstance) {
    // read VTS from register settings
    u8 temp1, temp2;
    u16  VTS;
    ov5640_read(IicInstance, 0x380e, &temp1);
    ov5640_read(IicInstance, 0x380f, &temp2);
    VTS = temp1;    
    VTS=(temp1<<8) + temp2;
    return VTS;
}



int ov5640_get_shutter(XIicPs *IicInstance) {
    // read shutter ,in number of line period       
    u8 shutter,temp[3];
    ov5640_read(IicInstance, 0x3500, &temp[0]);
    ov5640_read(IicInstance, 0x3501, &temp[1]);
    ov5640_read(IicInstance, 0x3502, &temp[2]);

    shutter = temp[0] & 0x0f;
    shutter = (shutter << 8) + temp[1];
    shutter = (shutter << 4) + (temp[2] >> 4);
    
    
    return shutter;
}

int ov5640_set_shutter(XIicPs *IicInstance,int shutter) {
    // read shutter ,in number of line period
    int Status;
    u8 temp;
    shutter = shutter & 0xffff;

    temp = shutter & 0x0f;
    temp = temp << 4;
    Status = ov5640_write(IicInstance, 0x3502, temp);
    if (Status != XST_SUCCESS)
      {
        xil_printf("set_shutter failed %d\r\n", Status);
        return Status;        
      }

    temp = shutter & 0xfff;
    temp = temp >> 4;
    ov5640_write(IicInstance, 0x3501, temp);
    if (Status != XST_SUCCESS)
      {
        xil_printf("set_shutter failed %d\r\n", Status);
        return Status;        
      }
    temp = shutter >> 12;
    ov5640_write(IicInstance, 0x3500, temp);
    if (Status != XST_SUCCESS)
      {
        xil_printf("set_shutter failed %d\r\n", Status);
        return Status;        
      }
    
    return Status;
}

int ov5640_get_gain16(XIicPs *IicInstance)
{
    // read gain, 16 = 1x
    u8 gain16, temp;

    ov5640_read(IicInstance, 0x350a, &temp);
    gain16 = temp & 0x03;

    ov5640_read(IicInstance, 0x350b, &temp);
    gain16 = (gain16 << 8) + temp;
    
    return gain16;
}

int ov5640_set_gain16(XIicPs *IicInstance,u8 gain16)
{
//write gain, 16 = 1x
    int temp,Status;
    gain16 = gain16 & 0x3ff;

    temp= gain16 & 0xff;
    Status=ov5640_write(IicInstance, 0x350b, temp);
    if (Status != XST_SUCCESS)
      {
        xil_printf("set_gain16 failed %d\r\n", Status);
        return Status;        
      }
    temp= gain16>>8;
    Status = ov5640_write(IicInstance, 0x350a, temp);
    if (Status != XST_SUCCESS)
      {
        xil_printf("set_gain16 failed %d\r\n", Status);
        return Status;        
      }
    return Status;
}

int ov5640_get_light_frequency(XIicPs *IicInstance)
{
    // get banding filter value
    u8 temp, temp1, light_frequency;
    ov5640_read(IicInstance, 0x3c01, &temp);
    
    if (temp & 0x80) {
        //manual
        ov5640_read(IicInstance, 0x3c00, &temp1);
        if(temp1 & 0x04) {
            //50Hz
            light_frequency = 50;
        }
        else {
            // 60Hz
            light_frequency = 60;
        }
    }
    else {
        // auto
        ov5640_read(IicInstance, 0x3c0c, &temp1);
        if(temp1 & 0x01) {
            //50Hz
            light_frequency = 50;
        }
        else {
          // 60Hz
          light_frequency = 60;
        }
    }
    return light_frequency;
}

void ov5640_set_bandingfilter(XIicPs *IicInstance) {
    u16 preview_HTS,preview_VTS,preview_sysclk;
    u16 band_step60, max_band60, band_step50, max_band50;

    // read preview PCLK;
    preview_sysclk = ov5640_get_sysclk(IicInstance);
    // read preview HTS
    preview_HTS = ov5640_get_HTS(IicInstance);

    // read preview VTS
    preview_VTS = ov5640_get_VTS(IicInstance);

    // calculate banding filter
    // 60Hz
    band_step60 = preview_sysclk *100 / preview_HTS  / 120;
    ov5640_write(IicInstance, 0x3a0a, band_step60 >> 8);
    ov5640_write(IicInstance, 0x3a0b, band_step60 & 0xff);

    max_band60 = (u16)((preview_VTS - 4) / band_step60);
    ov5640_write(IicInstance, 0x3a0b, max_band60);

    // 50Hz
    band_step50 = preview_sysclk *100 / preview_HTS  /100;
    ov5640_write(IicInstance, 0x3a08, band_step50 >> 8);
    ov5640_write(IicInstance, 0x3a09, band_step50 & 0xff);

    max_band50 = (u16)((preview_VTS - 4) / band_step50);
    ov5640_write(IicInstance, 0x3a0b, max_band50);

    }



int sensor_init(XIicPs *IicInstance)
{
    u8 sensor_id[2];
    u8  read_reg;
    u16 read_buf;
        
        
	ov5640_read(IicInstance, 0x300A, &sensor_id[0]);
	ov5640_read(IicInstance, 0x300B, &sensor_id[1]);

	if (sensor_id[0] != 0x56 || sensor_id[1] != 0x40)
	{
		xil_printf("Not ov5640 id, %x %x\r\n", sensor_id[0], sensor_id[1]);
		return 0 ;
	}
	else
	{
		xil_printf("Got ov5640 id, %x %x\r\n", sensor_id[0], sensor_id[1]);
    }
    
    //[1]=0 System input clock from pad; Default read = 0x11
	ov5640_write(IicInstance,0x3103,0x11);
	//[7]=1 Software reset; [6]=0 Software power down; Default=0x02
	ov5640_write(IicInstance,0x3008, 0x82);
	usleep(10000);

    //[7]=0 Software reset; [6]=1 Software power down; Default=0x02
    ov5640_write(IicInstance, 0x3008, 0x42);
    
    sensor_write_array(IicInstance, sensor_init_data);
    xil_printf("sensor_init_data");
    usleep(1000000);

    sensor_write_array(IicInstance, cfg_720p_60fps_raw8);
    xil_printf("cfg_720p_60fps");
    ov5640_auto_focus(IicInstance);
    //ov5640_set_bandingfilter(IicInstance);
   
// #if P1080 == 1
// 	//1080p 30fps
// 	sensor_write_array(IicInstance,cfg_1080p_30fps);
// #else
// 	//720p 60fps
// 	sensor_write_array(IicInstance,cfg_702p_60fps);
// #endif
	sensor_write_array(IicInstance,cfg_advanced_awb);
    
	//[7]=0 Software reset; [6]=0 Software power down; Default=0x02
    ov5640_write(IicInstance, 0x3008, 0x02);
    ov5640_write(IicInstance, 0x4202, 0x00);    
	ov5640_read(IicInstance,0x3008, &read_reg);
    xil_printf("Got ov5640 data, 0x3008 , %x \r\n", read_reg);

    read_buf=ov5640_get_light_frequency(IicInstance);
    xil_printf("light_frequency, %d \r\n", read_buf);

    read_buf = ov5640_get_sysclk(IicInstance);
   // xil_printf("sysclk, %d \r\n", read_buf);

    read_buf = ov5640_get_HTS(IicInstance);
    xil_printf("HTS, %d \r\n", read_buf);

    read_buf = ov5640_get_VTS(IicInstance);
    xil_printf("VTS, %d \r\n", read_buf);

    
    // ov5640_read(IicInstance,0x3003, &read_buf);
    // xil_printf("mipi reset,0x3003 %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3007, &read_buf);
    // xil_printf("mipi clk, 0x3007,%x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3034, &read_buf);
    // xil_printf("mipi clk, 0x3034,%x \r\n", read_buf);

    // ov5640_read(IicInstance,0x4800, &read_buf);
	// xil_printf("mipi Idle status,0x4800, %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3800, &read_buf);
    // xil_printf("X address start high byte,0x3800, %x \r\n", read_buf);


    // ov5640_read(IicInstance,0x3801, &read_buf);
    // xil_printf("X address start low byte,0x3801, %x \r\n", read_buf);
    
    // ov5640_read(IicInstance,0x3802, &read_buf);
    // xil_printf("Y address start high byte,0x3802, %x \r\n", read_buf);


    // ov5640_read(IicInstance,0x3803, &read_buf);
    // xil_printf("Y address start low byte,0x3803, %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3804, &read_buf);
    // xil_printf("X address end high byte,0x3804, %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3805, &read_buf);
    // xil_printf("X address end low byte,0x3805, %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3806, &read_buf);
    // xil_printf("Y address end high byte,0x3806, %x \r\n", read_buf);

    // ov5640_read(IicInstance,0x3807, &read_buf);
    // xil_printf("Y address end low byte,0x3807, %x \r\n", read_buf);

    // // ov5640_read(IicInstance,0x3803, &read_buf);
    // // xil_printf("Y address start low byte,0x3802, %d \r\n", read_buf);

    // // ov5640_read(IicInstance,0x3803, &read_buf);
    // // xil_printf("Y address start low byte,0x3802, %d \r\n", read_buf);

    // // ov5640_read(IicInstance,0x3803, &read_buf);
    // // xil_printf("Y address start low byte,0x3802, %d \r\n", read_buf);
    
	return 0;
}