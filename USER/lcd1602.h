#ifndef __LCD1602_H
#define __LCD1602_H

#include "stm32f10x.h"

// ========== 根据你的接线图重新定义 ==========
#define LCD1602_RS     GPIO_Pin_7     // PB7
#define LCD1602_E      GPIO_Pin_6     // PB6
#define LCD1602_D0     GPIO_Pin_4     // PB4
#define LCD1602_D1     GPIO_Pin_3     // PB3
#define LCD1602_D2     GPIO_Pin_15    // PA15
#define LCD1602_D3     GPIO_Pin_12    // PA12
#define LCD1602_D4     GPIO_Pin_11    // PA11
#define LCD1602_D5     GPIO_Pin_9     // PA9
#define LCD1602_D6     GPIO_Pin_8     // PA8
#define LCD1602_D7     GPIO_Pin_15    // PB15

#define LCD1602_RS_PORT GPIOB
#define LCD1602_E_PORT  GPIOB
#define LCD1602_D0_PORT GPIOB
#define LCD1602_D1_PORT GPIOB
#define LCD1602_D2_PORT GPIOA
#define LCD1602_D3_PORT GPIOA
#define LCD1602_D4_PORT GPIOA
#define LCD1602_D5_PORT GPIOA
#define LCD1602_D6_PORT GPIOA
#define LCD1602_D7_PORT GPIOB

// ========== 函数声明 ==========
void LCD1602_Init(void);
void LCD1602_WriteCmd(uint8_t cmd);
void LCD1602_WriteData(uint8_t dat);
void LCD1602_SetAddress(unsigned char x, unsigned char y);
void DispStrAt(unsigned char *str, unsigned char x, unsigned char y);
void LCD1602_ShowChar(unsigned char x, unsigned char y, unsigned char dat);
void DispDecSpaceAt(unsigned long Num, unsigned char x, unsigned char y, unsigned char Len);

#endif