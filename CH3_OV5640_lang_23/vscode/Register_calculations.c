#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TARGET_SYSCLK 8400
#define TARGET_P_CLK 7425
#define XVCLK 2400
#define sclk_EPSILON 1 // 允许的误差范围
#define PCLK_EPSILON 1000
#define BIT_DIV   2
#define  mipi_num_data_lanes 2
// 结构体用于保存符合条件的值的组合
typedef struct {
    int Pll_rdiv;
    int Bit_div;
    int pclk_div;
    int p_div;
    int Mipi_div;
    float Multiplier;
    float PreDiv;
    int SysDiv;
    int sclk_rdiv;
} VariableCombination;

void reg_calculation (VariableCombination *combinations,float PCLk);
int main() {
    int Pll_rdiv, Bit_div, pclk_div, p_div, Mipi_div, SysDiv, sclk_rdiv;
    float Multiplier, PreDiv;
    float result, sysclk,MIPI_clk,MIPI_clk_pre;
    VariableCombination *combinations = NULL; // 保存符合条件的值的组合的数组
    int num_combinations = 0; // 数组中值的数量
    int max_combinations = 900; // 数组的初始大小

    Bit_div = BIT_DIV;
    if (Bit_div != 2 && Bit_div != 2.5 )
     {  
        Bit_div = 1;
     }

 combinations = (VariableCombination *)malloc(max_combinations * sizeof(VariableCombination));

    for (Pll_rdiv = 1; Pll_rdiv <= 2; Pll_rdiv++) {
        //for (pclk_div = 1; pclk_div <= 8; pclk_div *= 2) {
            pclk_div = 1;
            for (p_div = 1; p_div <= 16; p_div++) {
                Mipi_div = p_div; // 根据约束条件 P_div = MIPI_div
                for (Multiplier = 21; Multiplier <= 252; Multiplier++) {
                    float PreDivValues[] = {1, 2, 3, 4, 1.5, 6, 2.5, 8};
                    for (int i = 0; i < sizeof(PreDivValues) / sizeof(PreDivValues[0]); i++) {
                        PreDiv = PreDivValues[i];
                        if (Multiplier / PreDiv >= 21 && Multiplier / PreDiv <= 74) {
                            for (SysDiv = 1; SysDiv <= 16; SysDiv++) {
                                    sclk_rdiv = 2;
                                // int sclk_rdiv_values[] = {1, 2, 4, 8};
                                // for (int j = 0; j < sizeof(sclk_rdiv_values) / sizeof(sclk_rdiv_values[0]); j++) {
                                //     sclk_rdiv = sclk_rdiv_values[j];
                                    result = (float)(Pll_rdiv * Bit_div * pclk_div ) ;
                                    // 计算sysclk
                                    sysclk = XVCLK * Multiplier / PreDiv / SysDiv / Pll_rdiv / Bit_div / sclk_rdiv;
                                    // 检查sysclk是否接近8400
                                    if (fabs(sysclk - TARGET_SYSCLK) < sclk_EPSILON && result >= 4 && result <= 5) {
                                        // 将符合条件的组合保存到数组中
                                        if (num_combinations >= max_combinations) {
                                            // 如果数组已满，重新分配内存
                                            max_combinations *= 2;
                                            combinations = realloc(combinations, max_combinations * sizeof(VariableCombination));
                                        }
                                        // 添加新的组合到数组中
                                        combinations[num_combinations].Pll_rdiv = Pll_rdiv;
                                        combinations[num_combinations].Bit_div = Bit_div;
                                        combinations[num_combinations].pclk_div = pclk_div;
                                        combinations[num_combinations].p_div = p_div;
                                        combinations[num_combinations].Mipi_div = Mipi_div;
                                        combinations[num_combinations].Multiplier = Multiplier;
                                        combinations[num_combinations].PreDiv = PreDiv;
                                        combinations[num_combinations].SysDiv = SysDiv;
                                        combinations[num_combinations].sclk_rdiv = sclk_rdiv;
                                        num_combinations++;
                                    }
                                
                            }
                        }
                    }
                }
            }
        
    }

    int j = 0;
    // 遍历数组中的值，计算P_clk并找出满足条件的组合
    for (int i = 0; i < num_combinations; i++) {
        float P_clk = XVCLK * combinations[i].Multiplier / combinations[i].PreDiv / combinations[i].SysDiv /
                      combinations[i].Pll_rdiv / combinations[i].Bit_div / combinations[i].pclk_div /
                      combinations[i].p_div;
        if (fabs(P_clk - TARGET_P_CLK) < PCLK_EPSILON) {
            sysclk = XVCLK * combinations[i].Multiplier / combinations[i].PreDiv / combinations[i].SysDiv / combinations[i].Pll_rdiv / combinations[i].Bit_div / combinations[i].sclk_rdiv;
            MIPI_clk = XVCLK * combinations[i].Multiplier / combinations[i].PreDiv / combinations[i].Mipi_div / 2;
            if(j == 0)
            {
                MIPI_clk_pre = MIPI_clk;
            }
            if (MIPI_clk <= MIPI_clk_pre){
                printf("Combination %d: Pll_rdiv=%d, Bit_div=%d, pclk_div=%d, p_div=%d, Mipi_div=%d, Multiplier=%.2f, PreDiv=%.2f, SysDiv=%d, sclk_rdiv=%d, P_clk=%.2f,sys_clk=%.2f,MIPI_clk=%.2f\n",
                       i+1, combinations[i].Pll_rdiv, combinations[i].Bit_div, combinations[i].pclk_div,
                       combinations[i].p_div, combinations[i].Mipi_div, combinations[i].Multiplier,
                       combinations[i].PreDiv, combinations[i].SysDiv, combinations[i].sclk_rdiv, P_clk, sysclk,MIPI_clk);
                       reg_calculation (&combinations[i],P_clk);
                MIPI_clk_pre = MIPI_clk;
            }
            j++;
        }
    }

    // 释放数组内存
    free(combinations);

    return 0;
}

void reg_calculation (VariableCombination *combinations,float P_clk)
{
    float pclk_period;
    
    //{0x3034, 0x3035,0x3037,0x3036,0x3108};
    typedef struct 
    {  
        int reg_3034;
        int reg_3035;
        int reg_3036;
        int reg_3037;
        int reg_3108;
    }REG_value;

    REG_value reg_value;// //
    
    reg_value.reg_3034 = 0x18;
    reg_value.reg_3035 = 0x11;
    reg_value.reg_3036 = 0x38;
    reg_value.reg_3037 = 0x11;
    reg_value.reg_3108 = 0x01;
        

    //0x3037[3:0]
    reg_value.reg_3037 = reg_value.reg_3037 & 0xf0;
    if(combinations->PreDiv == 1.5)
        reg_value.reg_3037 = reg_value.reg_3037|0x05;
    if(combinations->PreDiv == 2.5)
        reg_value.reg_3037 = reg_value.reg_3037|0x07;
    else reg_value.reg_3037 = reg_value.reg_3037|((int)(combinations->PreDiv));
    //0x3037[7] 3036
    if(combinations->Multiplier >= 128)
    { 
         reg_value.reg_3037 =reg_value.reg_3037 | 0x80;
    }    
    reg_value.reg_3036 = combinations->Multiplier;
    
    //0x3035
    reg_value.reg_3035 = reg_value.reg_3035 & 0x00;
    if(combinations->SysDiv == 16)
    { 
        reg_value.reg_3035 = reg_value.reg_3035 & 0x0f;
    }    
    else reg_value.reg_3035 = reg_value.reg_3035|((combinations->SysDiv)<<4);
    
    if(combinations->Mipi_div == 16)
    { 
        reg_value.reg_3035 = reg_value.reg_3035 & 0xf0;
    }    
    else reg_value.reg_3035 = reg_value.reg_3035|(combinations->Mipi_div);

    //3037[4]
    if(combinations->Pll_rdiv == 2)
    { 
        reg_value.reg_3037 = reg_value.reg_3037 | 0x10;
    }    
    else reg_value.reg_3037 = reg_value.reg_3037 & 0xef;

    //3034
    if(combinations->Bit_div == 2.5)
    { 
        reg_value.reg_3034 = 0x1A;
    }    
    else if(combinations->Bit_div == 2)
    { 
        reg_value.reg_3034 = 0x18;
    }

    //3108[5:4][1:0]
    int  Pclk_div_map[9]={0};
    Pclk_div_map[1]=0;
    Pclk_div_map[2]=1;
    Pclk_div_map[4]=2;
    Pclk_div_map[8]=3;
    reg_value.reg_3108 = reg_value.reg_3108 & 0xcc;
    reg_value.reg_3108 = reg_value.reg_3108|(Pclk_div_map[combinations->pclk_div]<<4);
    reg_value.reg_3108 = reg_value.reg_3108|(Pclk_div_map[combinations->sclk_rdiv]);

    pclk_period = (unsigned) ((1000000000UL + P_clk/2UL) / P_clk)/10000;
    pclk_period = pclk_period * mipi_num_data_lanes;
    printf("Combination_reg :  0x3034:0x%x, 0x3035:0x%x, 0x3036:0x%x, 0x3037:0x%x, 0x3108:0x%x , pclk_period : %.2f \n\n", reg_value.reg_3034,reg_value.reg_3035,reg_value.reg_3036,reg_value.reg_3037,reg_value.reg_3108,pclk_period);

}