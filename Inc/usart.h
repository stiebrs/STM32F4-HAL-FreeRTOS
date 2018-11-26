#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"

extern UART_HandleTypeDef huart1, huart6;

extern void _Error_Handler(char *, int);

void MX_USART1_UART_Init(void);
void MX_USART6_UART_Init(void);

#ifdef __cplusplus
}
#endif
#endif
