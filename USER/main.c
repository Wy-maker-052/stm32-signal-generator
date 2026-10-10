#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "ad9833.h"
#include "lcd1602.h"

// 全局变量
unsigned char k_num = 0;           // 0: Set f, 1: Set A, 2: Set B
unsigned long BujinData = 10;      // 步长初始值
unsigned long FreqData = 10000;    // 频率初始值 10kHz
unsigned int AmpData = 39;         // 幅度初始值 (≈12 * 3.3)
unsigned char WaveModle = 1;       // 1: SIN, 0: TRI, 2: SQU
unsigned char flag_key = 0;
unsigned char flag_k1 = 0, flag_k2 = 0, flag_k3 = 0, flag_k4 = 0;

/*************************************************
函数: void RCC_Configuration(void)
功能: 系统时钟配置（72MHz）
**************************************************/
void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);

        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);

        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 8MHz * 9 = 72MHz
        RCC_PLLCmd(ENABLE);

        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) { }

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08) { }
    }

    // 启用所有相关GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
}

/*************************************************
函数: void GPIO_Configuration(void)
功能: 按键 GPIO 初始化（PB0, PB1, PB10, PB11）
**************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*************************************************
函数: void Key(void)
功能: 按键扫描（K1: +, K2: -, K3: 切换模式, K4: 切换波形）
**************************************************/
void Key(void)
{
    // K1 (PB0) - +
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
    {
        delay_ms(5);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET && flag_k1 == 0)
        {
            flag_key = 1;
            flag_k1 = 1;
        }
    }
    else flag_k1 = 0;

    // K2 (PB1) - -
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET)
    {
        delay_ms(5);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET && flag_k2 == 0)
        {
            flag_key = 2;
            flag_k2 = 1;
        }
    }
    else flag_k2 = 0;

    // K3 (PB10) - 切换设置模式
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET)
    {
        delay_ms(5);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET && flag_k3 == 0)
        {
            flag_key = 3;
            flag_k3 = 1;
        }
    }
    else flag_k3 = 0;

    // K4 (PB11) - 切换波形
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_RESET)
    {
        delay_ms(5);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_RESET && flag_k4 == 0)
        {
            flag_key = 4;
            flag_k4 = 1;
        }
    }
    else flag_k4 = 0;
}

/*************************************************
主函数
**************************************************/
int main(void)
{
    // 字符串定义
    unsigned char dat1[] = "Set f";
    unsigned char dat2[] = "Sin";
    unsigned char dat3[] = "HZ";
    unsigned char dat4[] = "AM=";
    unsigned char dat5[] = "Set A";
    unsigned char dat6[] = "Set B";
    unsigned char dat7[] = "Tri";
    unsigned char dat8[] = "Squ";
    unsigned char dat9[] = "KHZ";

    // 系统初始化
    RCC_Configuration();
    GPIO_Configuration();
    delay_init();
    delay_ms(500);

    // LCD 初始化
    LCD1602_Init();
    LCD1602_WriteCmd(0x01);     // 清屏
    delay_ms(10);

    // ========== 初始显示 ==========
    // 第0行
    DispStrAt(dat5, 0, 0);                // "Set A"
    DispStrAt("   ", 6, 0);               // 清空波形区
    DispStrAt(dat2, 6, 0);                // "Sin"
    DispStrAt(" B=", 9, 0);               // " B="
    DispDecSpaceAt(BujinData, 13, 0, 3);  // 步长 (13,0)，宽3位

    // 第1行
    DispStrAt("       ", 0, 1);           // 清空频率区
    DispDecSpaceAt(FreqData, 0, 1, 6);    // 频率 (0,1)，宽6位
    DispStrAt(dat3, 6, 1);                // "HZ"
    DispStrAt(" ", 8, 1);                 // 间隔
    DispStrAt(dat4, 9, 1);                // "AM="
    DispDecSpaceAt((unsigned int)(AmpData / 3.3f), 12, 1, 3); // 幅度等级 (12,1)

    // AD9833 初始化
    AD9833_Init_GPIO(); // 必须先初始化GPIO
    AD9833_Write(0x0100); // 复位AD9833，确保从干净状态开始
    delay_ms(1);
	AD9833_WaveSeting(1000, 0, SIN_WAVE, 0);
	AD9833_AmpSet(200); 
//    AD9833_WaveSeting(FreqData, 0, SIN_WAVE, 0); // 设置初始波形
//    AD9833_AmpSet(AmpData);

    // 主循环
    while (1)
    {
        Key();

        // === K3: 切换设置模式 ===
        if (flag_key == 3)
        {
            flag_key = 0;
            k_num = (k_num + 1) % 3;
            DispStrAt("    ", 0, 0); // 清空前4列
            if (k_num == 0) DispStrAt(dat1, 0, 0); // "Set f"
            else if (k_num == 1) DispStrAt(dat5, 0, 0); // "Set A"
            else if (k_num == 2) DispStrAt(dat6, 0, 0); // "Set B"
        }

        // === K4: 切换波形 ===
        else if (flag_key == 4)
        {
            flag_key = 0;
            WaveModle = (WaveModle + 1) % 3;
            DispStrAt("    ", 6, 0); // 清空 4 个字符（5-8列）
            if (WaveModle == 1)
            {
                AD9833_WaveSeting(FreqData, 0, SIN_WAVE, 0);
                DispStrAt(dat2, 6, 0); // "Sin"
            }
            else if (WaveModle == 0)
            {
                AD9833_WaveSeting(FreqData, 0, TRI_WAVE, 0);
                DispStrAt(dat7, 6, 0); // "Tri"
            }
            else if (WaveModle == 2)
            {
                AD9833_WaveSeting(FreqData, 0, SQU_WAVE, 0);
                DispStrAt(dat8, 6, 0); // "Squ"
            }
        }
        // === 设置频率 (k_num == 0) ===
        if (k_num == 0 && (flag_key == 1 || flag_key == 2))
        {
            if (flag_key == 1) // +
            {
                if (FreqData < 12000000) FreqData += BujinData;
            }
            else if (flag_key == 2) // -
            {
                if (FreqData > BujinData) FreqData -= BujinData;
                else FreqData = BujinData;
            }
            flag_key = 0;
            AD9833_WaveSeting(FreqData, 0, WaveModle, 0);

            // 刷新频率显示
            DispStrAt("       ", 0, 1);
            if (FreqData < 10000000)
            {
                DispDecSpaceAt(FreqData, 0, 1, 6);
                DispStrAt(dat3, 6, 1); // "HZ"
            }
            else
            {
                DispDecSpaceAt(FreqData / 1000, 0, 1, 5);
                DispStrAt("K", 5, 1);
                DispStrAt("HZ", 6, 1); // "KHZ"
            }
        }

        // === 设置幅度 (k_num == 1) ===
        else if (k_num == 1 && (flag_key == 1 || flag_key == 2))
        {
            unsigned int amp_level;  // 提前声明！

            if (flag_key == 1) // +
            {
                if (AmpData < 165) AmpData += 3;
            }
            else if (flag_key == 2) // -
            {
                if (AmpData > 3) AmpData -= 3;
            }
            flag_key = 0;
            AD9833_AmpSet(AmpData);

            amp_level = (unsigned int)(AmpData / 3.3f);  // 赋值
            DispStrAt("   ", 12, 1);
            DispDecSpaceAt(amp_level, 12, 1, 3);
        }
        // === 设置步长 (k_num == 2) ===
        else if (k_num == 2 && (flag_key == 1 || flag_key == 2))
        {
            if (flag_key == 1) // ×10
            {
                if (BujinData < 1000000) BujinData *= 10;
            }
            else if (flag_key == 2) // ÷10
            {
                if (BujinData > 10) BujinData /= 10;
            }
            flag_key = 0;
            DispStrAt("    ", 13, 0);
            DispDecSpaceAt(BujinData, 13, 0, 4);
        }

        delay_ms(10); // 主循环小幅延时，防抖
    }
}