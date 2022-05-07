#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define USART_DEBUG		USART1		//调试打印所使用的串口组
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	

void usart1_init(u32 bound);
void usart2_init(u32 bound);
void usart3_init(u32 bound);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);
void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);

#endif


