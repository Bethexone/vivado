

#include "xil_types.h"
#include "iic_init.h"
#include "xiicps.h"

// LT8618SX RGB ????HDMI??????????????

////#define   LT8618SX_ADR		0x76                    // IIC Address??If CI2CA pins are high(2800mV~3300mV)
//#define   LT8618SX_ADR 0x72                     // IIC Address??If CI2CA pins are low(0~400mV)

/*--------------------------------
   CI2CA            I2C address
   -----------------------
   (0~400mV)----------0x72(default)
   (400mV~800mV)------0x7a
   (800mV~1200mV)-----0x90
   (1200mV~1600mV)----0x92

   (2000mV~2400mV)----0x94
   (2400mV~2800mV)----0x7e
   (2800mV~3300mV)----0x76
   -----------------------------------*/


/***************************************************

   ???????IIC???0x72(0x76)?????¦Ëbit0???§Õ???¦Ë???????Linux????IIC?????¦Ë???§Õ???¦Ë??IIC???????????¦Ë??IIC??????0x39(0x3B).

   IIC ??????????100K

   ???LT8618SXB ??IIC ???????0x72;??????????GPIO ???¦ËLT8618SXB????????100ms?????????delay100ms????????LT8618SXB???????

   The lowest bit 0 of the IIC address 0x72 (0x76) of LT8618SXB is the read-write flag bit.
   In the case of Linux IIC, the highest bit is the read-write flag bit, The IIC address needs to be moved one bit to the right, and the IIC address becomes 0x39 (0x3B).

   IIC rate should not exceed 100K.

   If the IIC address of LT8618SXB is not 0x72, you need to reset LT8618SXB with the master GPIO, pull down 100ms, then pull up, delay 100ms, and initialize the LT8618SXB register.

 ****************************************************/

// RGB888 channel Swap
#define _TTL_RGB_	0x70
#define _TTL_RBG_	0x60

#define _TTL_GBR_	0x40
#define _TTL_GRB_	0x50

#define _TTL_BRG_	0x30
#define _TTL_BGR_	0x00

// #define _LT8618_HDCP_ // ??????HDMI ???????????????????

/****************************************************/
u8	I2CADR;

u16 hfp, hs_width, hbp, h_act, h_tal, v_act, v_tal, vfp, vs_width, vbp;

u32 CLK_Cnt;

u8	Use_DDRCLK; // 1: DDR mode; 0: SDR (normal) mode

#ifdef _Read_TV_EDID_

u8	Sink_EDID[256];

u8	Sink_EDID2[256];

#endif

u8 CLK_Num;

u8 VIC_Num;    // vic ,0x10: 1080P ;  0x04 : 720P ; 

 /**************************************/

enum {
	_32KHz = 0,
	_44d1KHz,
	_48KHz,

	_88d2KHz,
	_96KHz,
	_176Khz,
	_196KHz
};

u16 IIS_N[] =
{
	4096,                   // 32K
	6272,                   // 44.1K
	6144,                   // 48K
	12544,                  // 88.2K
	12288,                  // 96K
	25088,                  // 176K
	24576                   // 196K
};

u8 Sample_Freq[] =
{
	0x30,                   // 32K
	0x00,                   // 44.1K
	0x20,                   // 48K
	0x80,                   // 88.2K
	0xa0,                   // 96K
	0xc0,                   // 176K
	0xe0                    // 196K
};

//************************************/
static u8 LT8618SXB_RGB888_SDR_PLL_setting[12][3] =
{
	{ 0x00, 0xa8, 0xbb },   // < 50MHz
	{ 0x00, 0x94, 0xaa },   // 50 ~ 59M
	{ 0x01, 0xa8, 0xaa },   // 60 ~ 89M
	{ 0x02, 0xbc, 0xaa },   // 90 ~ 99M

	{ 0x02, 0x9e, 0x99 },   // 100 ~ 119M
	{ 0x03, 0xa8, 0x99 },   // 120 - 149M
	{ 0x04, 0xb2, 0x99 },   // 150 - 179M
	{ 0x05, 0xbc, 0x99 },   // 180 - 189M

	{ 0x05, 0x9e, 0x88 },   // 200 - 209M
	{ 0x06, 0xa3, 0x88 },   // 210 - 239M
	{ 0x07, 0xa8, 0x88 },   // 240 - 269M
	{ 0x08, 0xad, 0x88 },   // >= 270 M
};

static u8 LT8618SXB_RGB888_DDR_PLL_setting[6][3] =
{
	// ???????D CLK???§³??????
	//  720P 60Hz DDR, D_CLK is 37MHz, select _Less_than_50M
	//  1080P 60Hz DDR, D_CLK is 74.25MHz, select _Bound_60_90M.
	//  4K30Hz DDR, D_CLK is 148.5MHz, select _Bound_120_150M

	//	{0x00,0xd0,0xbb},// < 25MHz
	{ 0x00, 0xa8, 0xaa },   // 25 ~ 50M
	{ 0x00, 0x94, 0x99 },   // 50 ~ 59M
	{ 0x01, 0xa8, 0x99 },   // 60 ~ 89M

	{ 0x02, 0xbc, 0x99 },   // 90 ~ 99M
	{ 0x02, 0x9e, 0x88 },   // 100 ~ 119M
	{ 0x03, 0xa8, 0x88 },   // 120 - 149M
	//	{0x04,0xb2,0x88},// 150 - 179M

	//	{0x05,0xbc,0x88},// 180 - 209M
	//	{0x06,0xc6,0x88},// 210 - 239M
	//	{0x07,0xd0,0x88},// 240 - 269M
	//	{0x08,0xda,0x88},// >= 270 M
};

enum
{
	_Less_than_50M = 0x00,                          // SDR:480P/ 576P; DDR: 480P/ 576P
	_Bound_50_60M,
	_Bound_60_90M,                                  // SDR:720P 60/50Hz; DDR:1080P 60/50Hz
	_Bound_90_100M,

	_Bound_100_120M,
	_Bound_120_150M,                                // SDR:1080P 60/50Hz; DDR:4K30Hz
	_Bound_150_180M,
	_Bound_180_200M,

	_Bound_200_210M,
	_Bound_210_240M,
	_Bound_240_270M,
	_Greater_than_270M                              // SDR:4K30Hz
};

// #define CLK_Num _Bound_60_90M                       // SDR_720P60
// #define CLK_Num _Less_than_50M  // 640x480

extern int i2c_reg8_write(XIicPs *InstancePtr, char IIC_ADDR, char Addr, char Data);
extern char i2c_reg8_read(XIicPs *InstancePtr, char IIC_ADDR, char Addr);


#ifdef _LT8618_HDCP_
/***********************************************************/

void LT8618SX_HDCP_Init( XIicPs *InstancePtr, char IIC_ADDR )                     //luodexing
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x13, 0xfe );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x13, 0xff );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x14, 0x00 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x14, 0xff );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x85 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x1f );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x13, 0xfe );               // [7]=force_hpd, [6]=force_rsen, [5]=vsync_pol, [4]=hsync_pol,
	                                                // [3]=hdmi_mode, [2]=no_accs_when_rdy, [1]=skip_wt_hdmi
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x17, 0x0f );               // [7]=ri_short_read, [3]=sync_pol_mode, [2]=srm_chk_done,
	                                                // [1]=bksv_srm_pass, [0]=ksv_list_vld
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x05 );
	//i2c_reg8_write(0x15,0x65);// [7]=key_ddc_st_sel, [6]=tx_hdcp_en,[5]=tx_auth_en, [4]=tx_re_auth
}

/***********************************************************/

void LT8618SX_HDCP_Enable( XIicPs *InstancePtr, char IIC_ADDR )                   //luodexing
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x14, 0x7f );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x14, 0xff );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x85 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x01 );               //disable HDCP
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x71 );               //enable HDCP
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x65 );               //enable HDCP
}

/***********************************************************/

void LT8618SX_HDCP_Disable( XIicPs *InstancePtr, char IIC_ADDR )                  //luodexing
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x85 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x45 );               //enable HDCP
}

#endif


/***********************************************************

***********************************************************/

void LT8918SX_Read_EDID( XIicPs *InstancePtr, char IIC_ADDR )
{
#ifdef _Read_TV_EDID_

	u8	i, j;
	u8	extended_flag = 0x00;

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x85 );
	//i2c_reg8_write(InstancePtr, IIC_ADDR, 0x02,0x0a); //I2C 100K
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x03, 0xc9 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x04, 0xa0 );               //0xA0 is EDID device address
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x05, 0x00 );               //0x00 is EDID offset address
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x06, 0x20 );               //length for read
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x14, 0x7f );

	for( i = 0; i < 8; i++ )                        // block 0 & 1
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x05, i * 32 );         //0x00 is EDID offset address
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x36 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x34 );           //0x31
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x37 );           //0x37
		Timer0_Delay1ms( 5 );                       // wait 5ms for reading edid data.
		if( i2c_reg8_read(InstancePtr, IIC_ADDR, 0x40 ) & 0x02 )      //KEY_DDC_ACCS_DONE=1
		{
			if( i2c_reg8_read(InstancePtr, IIC_ADDR, 0x40 ) & 0x50 )  //DDC No Ack or Abitration lost
			{
				printf( "\r\nread edid failed: no ack" );
				goto end;
			}else
			{
				printf( "\r\n" );
				for( j = 0; j < 32; j++ )
				{
					Sink_EDID[i * 32 + j] = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x83 );
					printf( " ", Sink_EDID[i * 32 + j] );

					//	edid_data = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x83 );

					if( ( i == 3 ) && ( j == 30 ) )
					{
						//	extended_flag = edid_data & 0x03;
						extended_flag = Sink_EDID[i * 32 + j] & 0x03;
					}
					//	printf( "%02bx,", edid_data );
				}
				if( i == 3 )
				{
					if( extended_flag < 1 ) //no block 1, stop reading edid.
					{
						goto end;
					}
				}
			}
		}else
		{
			printf( "\r\nread edid failed: accs not done" );
			goto end;
		}
	}

	if( extended_flag < 2 )                         //no block 2, stop reading edid.
	{
		goto end;
	}

	for( i = 0; i < 8; i++ )                        //	// block 2 & 3
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x05, i * 32 );         //0x00 is EDID offset address
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x76 );           //0x31
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x74 );           //0x31
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x77 );           //0x37
		Timer0_Delay1ms( 5 );                       // wait 5ms for reading edid data.
		if( i2c_reg8_read(InstancePtr, IIC_ADDR, 0x40 ) & 0x02 )      //KEY_DDC_ACCS_DONE=1
		{
			if( i2c_reg8_read(InstancePtr, IIC_ADDR, 0x40 ) & 0x50 )  //DDC No Ack or Abitration lost
			{
				printf( "\r\nread edid failed: no ack" );
				goto end;
			}else
			{
				printf( "\r\n" );
				for( j = 0; j < 32; j++ )
				{
					Sink_EDID2[i * 32 + j] = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x83 );
					printf( " ", Sink_EDID2[i * 32 + j] );

					//	edid_data = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x83 );
					//	printf( "%02bx,", edid_data );
				}
				if( i == 3 )
				{
					if( extended_flag < 3 ) //no block 1, stop reading edid.
					{
						goto end;
					}
				}
			}
		}else
		{
			printf( "\r\nread edid failed: accs not done" );
			goto end;
		}
	}
	//printf("\r\nread edid succeeded, checksum = ",Sink_EDID[255]);
end:
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x03, 0xc2 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x1f );
#endif

}

/***********************************************************

***********************************************************/
/* void LT8618SX_Reset( void )
{
	GPIO_WriteLow( GPIOD, GPIO_PIN_2 );     // Low
	Delay_ms( 100 );
	GPIO_WriteHigh( GPIOD, GPIO_PIN_2 );    // High
	Delay_ms( 100 );
} */

//------------------------------------------------------------

void LT8618SX_Chip_ID( XIicPs *InstancePtr, char IIC_ADDR )
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xFF, 0x80 );                                           // register bank
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xee, 0x01 );

	xil_printf("\r\nRead LT8618SXB ID ");
	xil_printf("\r\nLT8618SXB Chip ID00 = 0x%02x",i2c_reg8_read(InstancePtr, IIC_ADDR, 0x00));//0x17
	xil_printf("\r\nLT8618SXB Chip ID01 = 0x%02x",i2c_reg8_read(InstancePtr, IIC_ADDR, 0x01));//0x02
	xil_printf("\r\nLT8618SXB Chip ID02 = 0x%02x",i2c_reg8_read(InstancePtr, IIC_ADDR, 0x02));//0xe1

}

/***********************************************************

***********************************************************/
void LT8618SX_TTL_Input_Analog(  XIicPs *InstancePtr, char IIC_ADDR )
{
	// TTL mode
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x81 );       // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x02, 0x66 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x0a, 0x06 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x15, 0x06 );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x4e, 0xa8 );       // for U2

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x1b, 0x77 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x1c, 0xEC );       // 25000
}

/***********************************************************

***********************************************************/
void LT8618SX_TTL_Input_Digtal(  XIicPs *InstancePtr, char IIC_ADDR )
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x0A, 0xC0 );

	// TTL_Input_Digtal
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );       // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x45, _TTL_RGB_ );  //RGB channel swap

	if( Use_DDRCLK == 1 )
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x4f, 0x80 );   //0x00;  0x40: dclk
	}else
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x4f, 0x40 );   //0x00;  0x40: dclk
	}

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x50, 0x00 );
}

//------------------------------------------------------------

void LT8618SXB_HDMI_TX_En( XIicPs *InstancePtr, char IIC_ADDR, u8 enable)
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x81 );
	if( enable )
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x30, 0xea );
	}else
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x30, 0x00 );
	}
}

/***********************************************************

***********************************************************/
void LT8618SX_Video_Check( XIicPs *InstancePtr, char IIC_ADDR )
{
	u8	temp;
	u16 H_value	   = 0x0000;
	u16 V_value	   = 0x0000;

	usleep(100000);

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );   //video check

	temp = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x70 );   //hs vs polarity

//	if( temp & 0x02 )
//	{
//		vs_pol = 1;
//	}
//	if( temp & 0x01 )
//	{
//		hs_pol = 1;
//	}

	vs_width = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x71 );

	hs_width = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x72 ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x73 );

	vbp	   = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x74 );
	vfp	   = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x75 );

	hbp = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x76 ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x77 );

	hfp = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x78 ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x79 );

	v_tal = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7a ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7b );

	h_tal = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7c ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7d );

	v_act = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7e ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x7f );

	h_act = i2c_reg8_read(InstancePtr, IIC_ADDR, 0x80 ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x81 );

	CLK_Cnt = ( i2c_reg8_read(InstancePtr, IIC_ADDR, 0x1d ) & 0x0f ) * 0x10000 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x1e ) * 0x100 + i2c_reg8_read(InstancePtr, IIC_ADDR, 0x1f );
	// Pixel CLK =	CLK_Cnt * 1000

	xil_printf("\r\nh_FrontPorch = %d",hfp);
	xil_printf("\r\nh_SyncWidth =  %d",hs_width);
	xil_printf("\r\nh_BackPorch =  %d",hbp);
	xil_printf("\r\nh_active =   %d",h_act);
	xil_printf("\r\nh_total =   %d",h_tal);
	xil_printf("\r\nv_FrontPorch =  %d",vfp);
	xil_printf("\r\nv_SyncWidth =  %d",vs_width);
	xil_printf("\r\nv_BackPorch =  %d",vbp);
	xil_printf("\r\nv_active =  %d",v_act);
	xil_printf("\r\nv_total =  %d",v_tal);
}

/***********************************************************

***********************************************************/
void LT8618SXB_PLL_config( XIicPs *InstancePtr, char IIC_ADDR, u8 Bound_CLK )
{
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x81 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x23, 0x40 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x24, 0x64 );                                                   //icp set

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2e, 0x01 );                                                   // 0x01
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2f, 0x10 );                                                   // 0x00
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x26, 0x55 );

	if( Use_DDRCLK )
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x4d, 0x05 );

		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x25, LT8618SXB_RGB888_DDR_PLL_setting[CLK_Num][0x00] );    ////0x05 //pre-divider3 ddr 02
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2c, LT8618SXB_RGB888_DDR_PLL_setting[CLK_Num][0x01] );    // 9e
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2d, LT8618SXB_RGB888_DDR_PLL_setting[CLK_Num][0x02] );    // 88

		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x27, 0x66 );                                               //0x60 //ddr 0x66
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x28, 0x88 );                                               // 0x88
	}else
	{
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x4d, 0x00 );

		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x25, LT8618SXB_RGB888_SDR_PLL_setting[CLK_Num][0x00] );    ////0x05 //pre-divider3 ddr 02
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2c, LT8618SXB_RGB888_SDR_PLL_setting[CLK_Num][0x01] );    // 9e
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x2d, LT8618SXB_RGB888_SDR_PLL_setting[CLK_Num][0x02] );    // 88

		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x27, 0x06 );                                               //0x60 //ddr 0x66
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x28, 0x00 );                                               // 0x88
	}

	// as long as changing the resolution or changing the input clock,	You need to configure the following registers.
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xde, 0x00 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xde, 0xc0 );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x16, 0xf1 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x16, 0xf3 );
}

/***********************************************************

***********************************************************/
void LT8618SXB_Audio_setting(  XIicPs *InstancePtr, char IIC_ADDR )
{
	// IIS Input
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );   // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xd6, 0x8e );   // bit7 = 0 : DVI output; bit7 = 1: HDMI output
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xd7, 0x04 );   // sync polarity

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x84 );   // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x06, 0x08 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x10 );   // SD0 channel selected

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x09, 0x00 );   // 0x00 :Left justified; default
	// 0x02 :Right justified;

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x0f, 0x0b + Sample_Freq[_48KHz] );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x34, 0xd4 );   //CTS_N / 2; 32bit
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x34, 0xd5 );	//CTS_N / 4; 16bit

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x35, (u8)( IIS_N[_48KHz] / 0x10000 ) );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x36, (u8)( ( IIS_N[_48KHz] & 0x00FFFF ) / 0x100 ) );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x37, (u8)( IIS_N[_48KHz] & 0x0000FF ) );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x3c, 0x21 );   // Null packet enable

//-------------------------------------------


/*		// SPDIF Input
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 );// register bank
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0xd6, 0x8e );
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0xd7, 0x80 );	//sync polarity

   i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x84 );// register bank
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x06, 0x0c );
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x07, 0x10 );

   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x0f, 0x0b + Sample_Freq[_48KHz]);

   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x34, 0xd4 );//CTS_N
   //	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x34, 0xd5 );	//CTS_N / 4; 16bit

   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x35, (u8)(IIS_N[_48KHz]/0x10000) );
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x36, (u8)((IIS_N[_48KHz]&0x00FFFF)/0x100) );
   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x37, (u8)(IIS_N[_48KHz]&0x0000FF) );

   i2c_reg8_write(InstancePtr, IIC_ADDR, 0x3c, 0x21 );	// Null packet enable

 */
}

// LT8618SXB only supports three color space convert: YUV422, yuv444 and rgb888.
// Color space convert of YUV420 is not supported.
void LT8618SXB_CSC_setting(  XIicPs *InstancePtr, char IIC_ADDR )
{
	// color space config
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x82 ); // register bank
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x08 );// YCbCr444 to RGB
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x18 );// YCbCr422 to RGB

//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x80 );// RGB to YCbCr444
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0xa0 );// RGB to YCbCr422
				  
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x10 );// YCbCr422 to YCbCr444
//	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x20 );// YCbCr444 to YCbCr422
				   
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xb9, 0x00 ); // No csc
}

/***********************************************************

***********************************************************/
void LT8618SXB_AVI_setting(  XIicPs *InstancePtr, char IIC_ADDR )
{
	//AVI
	u8	AVI_PB0	   = 0x00;
	u8	AVI_PB1	   = 0x00;
	u8	AVI_PB2	   = 0x00;


	/********************************************************************************
	   The 0x43 register is checksums,
	   changing the value of the 0x45 or 0x47 register,
	   and the value of the 0x43 register is also changed.
	   0x43, 0x44, 0x45, and 0x47 are the sum of the four register values is 0x6F.
	 *********************************************************************************/

//	VIC_Num = 0x04; // 720P 60; Corresponding to the resolution to be output
//	VIC_Num = 0x10;	// 1080P 60
//	VIC_Num = 0x1F;	// 1080P 50
//	VIC_Num = 0x5F;	// 4K30

//================================================================//

	// Please refer to function: void LT8618SXB_CSC_setting( void )


	/****************************************************
	   Because the color space of RGB888 signal is RGB,
	   if lt8618sxb does not do color space convert (no CSC),
	   the color space of output HDMI is RGB.
	 *****************************************************/

	AVI_PB1 = 0x10;                         // PB1,color space: YUV444 0x70;YUV422 0x30; RGB 0x10

//===============================================================//

	AVI_PB2 = 0x2A;                         // PB2; picture aspect rate: 0x19:4:3 ;     0x2A : 16:9

	AVI_PB0 = ( ( AVI_PB1 + AVI_PB2 + VIC_Num ) <= 0x6f ) ? ( 0x6f - AVI_PB1 - AVI_PB2 - VIC_Num ) : ( 0x16f - AVI_PB1 - AVI_PB2 - VIC_Num );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x84 );       // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x43, AVI_PB0 );    // PB0,avi packet checksum
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x44, AVI_PB1 );    // PB1,color space: YUV444 0x70;YUV422 0x30; RGB 0x10
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x45, AVI_PB2 );    // PB2;picture aspect rate: 0x19:4:3 ; 0x2A : 16:9
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x47, VIC_Num );    // PB4;vic ,0x10: 1080P ;  0x04 : 720P

//	i2c_reg8_write(InstancePtr, IIC_ADDR,0xff,0x84);
//	8618SXB hdcp1.4 ????????????hfp + 8410[5:0](rg_island_tr_res) ???????video de???????? ????????aude ????????????58??hdcp1.4 spec???????
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x10, 0x2c );       //data iland
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x12, 0x64 );       //act_h_blank

	//VS_IF, 4k 30hz need send VS_IF packet. Please refer to hdmi1.4 spec 8.2.3
	if( VIC_Num == 95 )
	{
//	   i2c_reg8_write(0xff,0x84);
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x3d, 0x2a );   //UD1 infoframe enable

		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x74, 0x81 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x75, 0x01 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x76, 0x05 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x77, 0x49 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x78, 0x03 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x79, 0x0c );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x7a, 0x00 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x7b, 0x20 );
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x7c, 0x01 );
	}else
	{
//	   i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff,0x84);
		i2c_reg8_write(InstancePtr, IIC_ADDR, 0x3d, 0x0a ); //UD1 infoframe disable
	}
}

/***********************************************************

***********************************************************/
void LT8618SXB_TX_Phy(  XIicPs *InstancePtr, char IIC_ADDR )
{
	// HDMI_TX_Phy
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x81 ); // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x30, 0xea );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x31, 0x44 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x32, 0x4a );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x33, 0x0b );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x34, 0x00 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x35, 0x00 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x36, 0x00 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x37, 0x44 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x3f, 0x0f );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x40, 0xb0 );   //0xa0 -- CLK tap0 swing
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x41, 0x68 );   //0xa0 -- D0 tap0 swing
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x42, 0x68 );   //0xa0 -- D1 tap0 swing
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x43, 0x68 );   //0xa0 -- D2 tap0 swing

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x44, 0x0a );
}

//******************************************************//

// Step:

// 1??Reset LT8618SX

// 2??LT8618SX Initial setting:

void LT8618SX_Initial(  XIicPs *InstancePtr, char IIC_ADDR )
{
//	I2CADR = LT8618SX_ADR;              // ????IIC???

	Use_DDRCLK = 0;                     // 1: DDR mode; 0: SDR (normal) mode

	// For example, rgb888 signal is 1280x720 60Hz resolution, pixel clk is 74.25mhz.
	// Parameters(CLK_Num) required by LT8618SXB_PLL_setting( void )
	if(Use_DDRCLK)
		{
		CLK_Num = _Less_than_50M; // DDR 720P40 37.125M 
		}
	else
		{
		CLK_Num = _Bound_60_90M; // SDR 720P40 74.25M
		}

	VIC_Num = 0x04;// 720P60; Parameters required by LT8618SXB_AVI_setting( void )
//	VIC_Num = 0x10; // 1080P 60
//	VIC_Num = 0x1F; // 1080P 50
//	VIC_Num = 0x5F; // 4K30;

//  With different resolutions, Vic has different values, Refer to the following list

/*************************************
   Resolution			VIC_Num
   --------------------------------------
   640x480				1
   720x480P 60Hz		2
   720x480i 60Hz		6

   720x576P 50Hz		17
   720x576i 50Hz		21

   1280x720P 24Hz		60
   1280x720P 25Hz		61
   1280x720P 30Hz		62
   1280x720P 50Hz		19
   1280x720P 60Hz		4

   1920x1080P 24Hz		32
   1920x1080P 25Hz		33
   1920x1080P 30Hz		34

   1920x1080i 50Hz		20
   1920x1080i 60Hz		5

   1920x1080P 50Hz		31
   1920x1080P 60Hz		16

   3840x2160 30Hz		95 // 4K30

   Other resolution 	0(default) // Such as 1024x768 / 800x600 / 1366x768.....

 **************************************/

//--------------------------------------------------------//	
//	LT8618SX_Reset( );

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );   // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xee, 0x01 );

	LT8618SX_Chip_ID(InstancePtr, IIC_ADDR );                // for debug

	// RST_PD_Init
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0xff, 0x80 );   // register bank
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x11, 0x00 );   //reset MIPI Rx logic.

	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x13, 0xf1 );
	i2c_reg8_write(InstancePtr, IIC_ADDR, 0x13, 0xf9 );   // Reset TTL video process

	LT8618SX_TTL_Input_Analog(InstancePtr, IIC_ADDR );

	LT8618SX_TTL_Input_Digtal(InstancePtr, IIC_ADDR );

//----------------------------------------------------------//

	//Wait for the signal to be stable and decide whether the delay is necessary according to the actual situation
	usleep( 1000000 );           // ?????????,???????????????????????

	LT8618SX_Video_Check(InstancePtr, IIC_ADDR );    // For debug

//=========================================================

	// PLL setting
	LT8618SXB_PLL_config(InstancePtr, IIC_ADDR, CLK_Num );

//=========================================================

	LT8618SXB_Audio_setting(InstancePtr, IIC_ADDR );
//-------------------------------------------

	// color space config
	LT8618SXB_CSC_setting(InstancePtr, IIC_ADDR );

//-------------------------------------------

#ifdef _LT8618_HDCP_
	LT8618SX_HDCP_Init( );
#endif

//-------------------------------------------

	//AVI
	LT8618SXB_AVI_setting(InstancePtr, IIC_ADDR );

//-------------------------------------------

	// HDMI_TX_Phy
	LT8618SXB_TX_Phy(InstancePtr, IIC_ADDR );

//-------------------------------------------

#ifdef _LT8618_HDCP_
	LT8618SX_HDCP_Enable( );
#endif

//-------------------------------------------

#ifdef _Read_TV_EDID_
	// This operation is not necessary. Read TV EDID if necessary.
	LT8918SX_Read_EDID( ); // Read TV  EDID
#endif
//-------------------------------------------
}

/************************************** The End Of File **************************************/
