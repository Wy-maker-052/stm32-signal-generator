#include "lcd1602.h"
#include "delay.h"

// ========== GPIO 初始化 ==========
void LCD1602_GPIO_Init_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    // 初始化 PA 上的数据线 (D2~D6)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LCD1602_D2 | LCD1602_D3 | LCD1602_D4 | LCD1602_D5 | LCD1602_D6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始化 PB 上的控制线和数据线 (RS, E, D0, D1, D7)
    GPIO_InitStructure.GPIO_Pin = LCD1602_RS | LCD1602_E | LCD1602_D0 | LCD1602_D1 | LCD1602_D7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// ========== 写命令 ==========
void LCD1602_WriteCmd(uint8_t cmd)
{
    GPIO_ResetBits(LCD1602_RS_PORT, LCD1602_RS);  // RS=0: 命令

    // 逐位设置数据线
    if (cmd & 0x01) GPIO_SetBits(LCD1602_D0_PORT, LCD1602_D0); else GPIO_ResetBits(LCD1602_D0_PORT, LCD1602_D0);
    if (cmd & 0x02) GPIO_SetBits(LCD1602_D1_PORT, LCD1602_D1); else GPIO_ResetBits(LCD1602_D1_PORT, LCD1602_D1);
    if (cmd & 0x04) GPIO_SetBits(LCD1602_D2_PORT, LCD1602_D2); else GPIO_ResetBits(LCD1602_D2_PORT, LCD1602_D2);
    if (cmd & 0x08) GPIO_SetBits(LCD1602_D3_PORT, LCD1602_D3); else GPIO_ResetBits(LCD1602_D3_PORT, LCD1602_D3);
    if (cmd & 0x10) GPIO_SetBits(LCD1602_D4_PORT, LCD1602_D4); else GPIO_ResetBits(LCD1602_D4_PORT, LCD1602_D4);
    if (cmd & 0x20) GPIO_SetBits(LCD1602_D5_PORT, LCD1602_D5); else GPIO_ResetBits(LCD1602_D5_PORT, LCD1602_D5);
    if (cmd & 0x40) GPIO_SetBits(LCD1602_D6_PORT, LCD1602_D6); else GPIO_ResetBits(LCD1602_D6_PORT, LCD1602_D6);
    if (cmd & 0x80) GPIO_SetBits(LCD1602_D7_PORT, LCD1602_D7); else GPIO_ResetBits(LCD1602_D7_PORT, LCD1602_D7);

    // 使能脉冲
    GPIO_SetBits(LCD1602_E_PORT, LCD1602_E);
    delay_us(2);   // E 脉冲宽度 >450ns，2μs 足够
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E);
    delay_us(50);  // 指令执行延时（>37μs）
}

// ========== 写数据 ==========
void LCD1602_WriteData(uint8_t dat)
{
    GPIO_SetBits(LCD1602_RS_PORT, LCD1602_RS);  // RS=1: 数据

    if (dat & 0x01) GPIO_SetBits(LCD1602_D0_PORT, LCD1602_D0); else GPIO_ResetBits(LCD1602_D0_PORT, LCD1602_D0);
    if (dat & 0x02) GPIO_SetBits(LCD1602_D1_PORT, LCD1602_D1); else GPIO_ResetBits(LCD1602_D1_PORT, LCD1602_D1);
    if (dat & 0x04) GPIO_SetBits(LCD1602_D2_PORT, LCD1602_D2); else GPIO_ResetBits(LCD1602_D2_PORT, LCD1602_D2);
    if (dat & 0x08) GPIO_SetBits(LCD1602_D3_PORT, LCD1602_D3); else GPIO_ResetBits(LCD1602_D3_PORT, LCD1602_D3);
    if (dat & 0x10) GPIO_SetBits(LCD1602_D4_PORT, LCD1602_D4); else GPIO_ResetBits(LCD1602_D4_PORT, LCD1602_D4);
    if (dat & 0x20) GPIO_SetBits(LCD1602_D5_PORT, LCD1602_D5); else GPIO_ResetBits(LCD1602_D5_PORT, LCD1602_D5);
    if (dat & 0x40) GPIO_SetBits(LCD1602_D6_PORT, LCD1602_D6); else GPIO_ResetBits(LCD1602_D6_PORT, LCD1602_D6);
    if (dat & 0x80) GPIO_SetBits(LCD1602_D7_PORT, LCD1602_D7); else GPIO_ResetBits(LCD1602_D7_PORT, LCD1602_D7);

    GPIO_SetBits(LCD1602_E_PORT, LCD1602_E);
    delay_us(2);
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E);
    delay_us(50);
}

// ========== 设置光标地址 ==========
void LCD1602_SetAddress(unsigned char x, unsigned char y)
{
    if (y == 0)
        LCD1602_WriteCmd(0x80 | x);          // 第一行
    else
        LCD1602_WriteCmd(0x80 | 0x40 | x);   // 第二行
}

// ========== 显示字符串 ==========
void DispStrAt(unsigned char *str, unsigned char x, unsigned char y)
{
    LCD1602_SetAddress(x, y);
    while (*str != '\0')
    {
        // 过滤非法字符：只显示可打印ASCII（32~126）
        if (*str >= 32 && *str <= 126) {
            LCD1602_WriteData(*str);
        } else {
            LCD1602_WriteData(' '); // 非法字符显示为空格
        }
        str++;
    }
}

// ========== 显示单个字符 ==========
void LCD1602_ShowChar(unsigned char x, unsigned char y, unsigned char dat)
{
    LCD1602_SetAddress(x, y);
    LCD1602_WriteData(dat);
}

// ========== 修复版：指定位置显示数字（左对齐，带空格填充） ==========
void DispDecSpaceAt(unsigned long Num, unsigned char x, unsigned char y, unsigned char Len)
{
    char buf[16];  // 足够大的缓冲区
    unsigned char i = 0, j;

    // 初始化缓冲区为全空格
    for (j = 0; j < 16; j++) {
        buf[j] = ' ';
    }

    if (Num == 0)
    {
        buf[0] = '0';
        i = 1;
    }
    else
    {
        // 将数字转换为字符串，注意边界检查
        unsigned long temp = Num;
        while (temp > 0 && i < 14)  // 留2个位置给 '\0' 和安全边界
        {
            buf[i++] = (temp % 10) + '0';
            temp /= 10;
        }
        
        // 反转字符串
        for (j = 0; j < i / 2; j++)
        {
            char temp_char = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = temp_char;
        }
    }

    // ?? 确保字符串以 '\0' 结尾
    buf[i] = '\0';

    LCD1602_SetAddress(x, y);

    // 输出前导空格（如果数字位数不足Len）
    for (j = 0; j < Len - i; j++)
    {
        LCD1602_WriteData(' ');
    }

    // 输出数字字符
    for (j = 0; j < i; j++)
    {
        if (buf[j] != '\0') {  // 额外安全检查
            LCD1602_WriteData(buf[j]);
        }
    }
}

// ========== LCD 初始化 ==========
void LCD1602_Init(void)
{
    LCD1602_GPIO_Init_Out();
    delay_ms(20);  // 上电稳定延时（>15ms）

    LCD1602_WriteCmd(0x38);  // 8位数据，2行，5x7点阵
    delay_us(50);
    LCD1602_WriteCmd(0x0C);  // 显示开，光标关
    delay_us(50);
    LCD1602_WriteCmd(0x06);  // 自动增址，不移动屏幕
    delay_us(50);
    LCD1602_WriteCmd(0x01);  // 清屏
    delay_ms(5);             // 清屏需 >1.64ms
}