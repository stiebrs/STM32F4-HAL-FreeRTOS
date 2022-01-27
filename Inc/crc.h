#ifndef __crc_H
#define __crc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"

extern CRC_HandleTypeDef hcrc;

extern void _Error_Handler(char *, int);

void MX_CRC_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ crc_H */
