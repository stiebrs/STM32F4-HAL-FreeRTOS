#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx_hal.h"
#include "main.h"

volatile uint8_t spiDone;
void SysTick_Handler(void);
void TIM6_DAC_IRQHandler(void);
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream4_IRQHandler(void);
void SPI5_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
