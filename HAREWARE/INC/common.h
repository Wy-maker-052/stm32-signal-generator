#ifndef __COMMON_H
#define __COMMON_H

//#include "bitband.h"





extern u8 USART_RxDat;       //接收数据
extern u8 USART_RxFlag;      //接收标志位
void Delay_Init(void);       //延时初始化函数
void Delay_ms(u16 ms);       //延时X毫秒函数
void Delay_us(u32 us);       //延时X微妙函数
void USART_Config(USART_TypeDef* USARTx,u32 Baud);  //串口初始化
u8 UartSendData(USART_TypeDef* USARTx,u8 ch);
void UartSendString(USART_TypeDef* USARTx,char *str);
#endif




