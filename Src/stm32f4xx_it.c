#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "cmsis_os.h"
#include "spi.h"

extern TIM_HandleTypeDef htim6;
extern SPI_HandleTypeDef hspi5;
extern osMessageQId xGyroQueue;
extern uint8_t rxbuf[3];
extern uint8_t txbuf[3];
extern osPoolId spi_pool;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void) {
  osSystickHandler();
}

void TIM6_DAC_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim6);
}

// SPI5 DMA receive done
void DMA2_Stream3_IRQHandler(void) {
	SPI_QueueItem_t *item = (SPI_QueueItem_t*)osPoolAlloc(spi_pool);
	item->value = rxbuf[1];
	item->source = "DMA IRQ";
	osMessagePut(xGyroQueue, (uint32_t) item, 0);

	HAL_DMA_IRQHandler(hspi5.hdmarx);
}

// SPI5 DMA transmit done
void DMA2_Stream4_IRQHandler(void) {
	HAL_DMA_IRQHandler(hspi5.hdmatx);
}

void SPI5_IRQHandler(void) {
	spiDone = 1;
	HAL_SPI_IRQHandler(&hspi5);
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_SET);
}
