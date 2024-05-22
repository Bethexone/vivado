#include <stdio.h>

int main()
{   
    int XVCLK = 2400;
    int pclk;
    long  pclk_period;
    int mipi_num_data_lanes = 2;
    // calculate sysclk
    unsigned int  temp1, temp2;
    int Multiplier, VCO, SysDiv,Mipi_div, Pll_rdiv, sclk_rdiv, pclk_div,p_div,sysclk,MIPI_clk,P_clk;
    float PreDiv,Bit_div;
    float PreDiv_map[]={1,1,2,3,4,1.5,6,2.5,8,1,1,1,1,1,1,1};
    int sclk_rdiv_map[] = {1, 2, 4, 8};
    int reg[]=     {0x3034, 0x3035,0x3036,0x3037,0x3108};
    int P720_raw10[]={0x1a,0x21,0x46,0x05,0x11,36};
    int P720_raw8[]={0x18,0x13,0x1c,0x01,0x11,29};
    int P_raw[]={0x18,0x12,0x1c,0x01,0x16,36};
// Combination 93: Pll_rdiv=1, Bit_div=2, pclk_div=2, p_div=2, Mipi_div=2, Multiplier=28.00, PreDiv=1.00, SysDiv=1, sclk_rdiv=4, P_clk=8400.00,sys_clk=4200.00,MIPI_clk=16800.00
// Combination_reg :  0x3034:0x18, 0x3035:0x12, 0x3036:0x1c, 0x3037:0x1, 0x3108:0x12 , pclk_period : 22.00
    int P720_raw8_of[]={0x18,0x11,0x13,0x54,0x01,22};
    int P1080_raw10[]={0x1a,0x21,0x05,0x69,0x11};
    int P1080_raw8[]={0x18,0x21,0x69,0x05,0x11,10};

//ov5640_read(IicInstance,0x3034, &temp1);
    temp1 = P_raw[0];
    temp2 = temp1 & 0x0f;
    if (temp2 == 8 || temp2 == 10) {
      Bit_div=temp2/(4.0);
    }
    else
      Bit_div=1;
    
//ov5640_read(IicInstance, 0x3035, &temp1);
    temp1=P_raw[1];
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

//ov5640_read(IicInstance, 0x3037, &temp1);
    temp1 =P_raw[3];
    temp2 =temp1>>7;
    PreDiv = PreDiv_map[temp1 & 0x0f];
    Pll_rdiv = ((temp1 >> 4) & 0x01) + 1;

//ov5640_read(IicInstance, 0x3036, &temp1);
    temp1 = P_raw[2];
    if (temp2)
      Multiplier = (temp1 >> 1) * 2;
    else Multiplier = temp1&0x7f;

//ov5640_read(IicInstance, 0x3108, &temp1);
    temp1 =P_raw[4];
    temp2 = temp1 & 0x03;
    sclk_rdiv = sclk_rdiv_map[temp2];
    temp2 = temp1 & 0x30;
    pclk_div = sclk_rdiv_map[temp2>>4];

    //3824[4:0]    

    VCO = XVCLK * Multiplier / PreDiv;
    sysclk = VCO / SysDiv / Pll_rdiv  / Bit_div / sclk_rdiv;
    //sysclk =  XVCLK * Multiplier / PreDiv/ SysDiv / Pll_rdiv  / Bit_div / sclk_rdiv/2;;
    MIPI_clk = VCO / SysDiv / Mipi_div / 2;
    //MIPI_clk = XVCLK * Multiplier / PreDiv/ SysDiv / Mipi_div / 2;
    P_clk = VCO / SysDiv / Pll_rdiv  / Bit_div / pclk_div / p_div;
    //P_clk = XVCLK * Multiplier / PreDiv/  SysDiv / Pll_rdiv  / Bit_div / pclk_div / p_div/2;

    pclk = sysclk * sclk_rdiv /Mipi_div;
    pclk_period = (unsigned) ((1000000000UL +  P_clk/2UL) / P_clk)/10000;
    pclk_period = pclk_period * mipi_num_data_lanes;

    
    printf("sysclk:%d  MIPI_clk:%d P_clk:%d pclk_period: %d", sysclk,MIPI_clk,P_clk,pclk_period);

// Combination 93: Pll_rdiv=1, Bit_div=2, pclk_div=2, p_div=2, Mipi_div=2, Multiplier=28.00, PreDiv=1.00, SysDiv=1, sclk_rdiv=4, P_clk=8400.00,sys_clk=4200.00,MIPI_clk=16800.00
// Combination_reg :  0x3034:0x18, 0x3035:0x12, 0x3036:0x1c, 0x3037:0x1, 0x3108:0x12 , pclk_period : 22.00
    return 0;
   
} 