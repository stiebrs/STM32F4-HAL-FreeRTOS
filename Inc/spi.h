#ifndef __spi_H
#define __spi_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"
#include "cmsis_os.h"


extern SPI_HandleTypeDef hspi5;
extern osMessageQId xGyroQueue;
extern uint8_t rxbuf[3];
extern uint8_t txbuf[3];
extern osPoolId spi_pool;

extern void _Error_Handler(char *, int);

typedef struct __SPI_QueueItem_t {
  uint32_t value;
  char *source;
} SPI_QueueItem_t;

void MX_SPI5_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ spi_H */
