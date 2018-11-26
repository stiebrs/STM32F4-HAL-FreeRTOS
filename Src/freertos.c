#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f4xx_it.h"
#include "usb_device.h"

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle, blinkyTaskHandle, gyroTaskHandle, gyroPrinterHandle;
osMessageQId xGyroQueue;
uint8_t rxbuf[3] = { 0x00, 0x00 };
uint8_t txbuf[3] = { 0x0F | 0x80, 0x00 }; // 0x0F is WHO_AM_I register, 0x80 read bit, should return 0b11010100 or 0xD4
osPoolDef(spi_pool, 10, SPI_QueueItem_t);
osPoolId spi_pool;

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void vBlinkyTask(void const * argument);
void vGyroTesterTask(void const * argument);
void vGyroPrinterTask(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

__attribute__((weak)) void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
	 task. It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()). If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */
}

__attribute__((weak)) void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
}

__attribute__((weak)) void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created. It is also called by various parts of the
	 demo application. If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
}

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
	spi_pool = osPoolCreate(osPool(spi_pool));
	osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 1000);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);


	HAL_Delay(3000);
	osThreadDef(blinkyTask, vBlinkyTask, osPriorityHigh, 4, 1000);
	blinkyTaskHandle = osThreadCreate(osThread(blinkyTask), NULL);

	osThreadDef(gyroPrinterTask, vGyroPrinterTask, osPriorityLow, 1, 1000);
	gyroPrinterHandle = osThreadCreate(osThread(gyroPrinterTask), NULL);

	osMessageQDef(gyroPrinterQueue, 10, SPI_QueueItem_t);	// 10 pointers
	xGyroQueue = osMessageCreate(osMessageQ(gyroPrinterQueue), NULL);

	// Put test data into the queue
	SPI_QueueItem_t *item = (SPI_QueueItem_t*) osPoolAlloc(spi_pool);
	item->value = 0x33;
	item->source = "TEST";
	osMessagePut(xGyroQueue, (uint32_t) item, 0);

	osThreadDef(gyroTask, vGyroTesterTask, osPriorityHigh, 0, 1000);
	gyroTaskHandle = osThreadCreate(osThread(gyroTask), NULL);

}

/* StartDefaultTask function which blinks LED2 every 1s */
void StartDefaultTask(void const * argument) {
	MX_USB_DEVICE_Init();
	for (;;) {
	}
}

/* vBlinkyTask function which blinks LED1 every 10s and outputs a string to UART */
void vBlinkyTask(void const * argument) {
	for (;;) {
		HAL_GPIO_TogglePin(LED_GREEN_Port, LED_GREEN_Pin);
		if (HAL_GPIO_ReadPin(LED_GREEN_Port, LED_GREEN_Pin) == GPIO_PIN_RESET) {
			printf("Blink!\r\n");
		}
		osDelay(5000);
	}
}

void vGyroTesterTask(void const * argument) {
	HAL_StatusTypeDef response = HAL_ERROR; // default to error, so we can see, if value actually gets updated by HAL

	/* Transceive data with gyro in blocking mode */
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_RESET);
	response = HAL_SPI_TransmitReceive(&hspi5, txbuf, rxbuf, 2, 1000);
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_SET);
	if (response == HAL_OK) {
		printf("Sent: %02x %02x Got: %02x %02x\r\n", txbuf[0], txbuf[1], rxbuf[0], rxbuf[1]);
	} else {
		printf("Got error response as %d\r\n", response);
	}

	/* Now do the same in DMA mode */
	memset(rxbuf, 0x00, sizeof rxbuf);
	printf("RX buffer reset to %02x %02x\r\n", rxbuf[0], rxbuf[1]);
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_RESET);
	response = HAL_SPI_TransmitReceive_DMA(&hspi5, txbuf, rxbuf, 2);
	if (response != HAL_OK) {
		printf("Got error response as %d\r\n", response);
	}

	/* Print fome stuff, just to keep CPU busy to show that it's actually DMA performing transmit */
	uint8_t state = HAL_SPI_GetState(&hspi5);
	while (state != HAL_SPI_STATE_READY) {
		state = HAL_SPI_GetState(&hspi5);
		printf("State is: %d\r\n", state);
	}
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_SET);
	printf("Sent via DMA: %02x %02x Got: %02x %02x\r\n", txbuf[0], txbuf[1], rxbuf[0], rxbuf[1]);

	/* Again, this time using interrupts */
	memset(rxbuf, 0x00, sizeof rxbuf);
	printf("RX buffer reset to %02x %02x\r\n", rxbuf[0], rxbuf[1]);
	state = HAL_SPI_GetState(&hspi5);
	spiDone = 0;
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_RESET);
	response = HAL_SPI_TransmitReceive_IT(&hspi5, txbuf, rxbuf, 2);
	while (state != HAL_SPI_STATE_READY) {
		state = HAL_SPI_GetState(&hspi5);
		printf("State is: %d\r\n", state);
	}
	if (response != HAL_OK) {
		printf("Got error response as %d\r\n", response);
	}

	while (spiDone != 1) {
		printf("Not done yet!\r\n");
	}
	printf("Sent via IT: %02x %02x Got: %02x %02x\r\n", txbuf[0], txbuf[1], rxbuf[0], rxbuf[1]);

	osThreadTerminate(gyroTaskHandle);
}

void vGyroPrinterTask(void const * argument) {
	osEvent event;
	uint8_t count = 0;
	SPI_QueueItem_t *item;
	while (1) {
		event = osMessageGet(xGyroQueue, osWaitForever);
		printf("Got %ld messages in queue\r\n", osMessageWaiting(xGyroQueue) + 1);
		while (event.status == osEventMessage) {
			item = (SPI_QueueItem_t *) event.value.p;
			count++;
			printf("Message %d: from %s: %lx\r\n", count, item->source, item->value);
			osPoolFree(spi_pool, item);
			event = osMessageGet(xGyroQueue, 1);
		}
		count = 0;
		osThreadYield();
	}
}
