#ifndef __tim_H
#define __tim_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;

extern void _Error_Handler(char *, int);

void MX_TIM1_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ tim_H */
