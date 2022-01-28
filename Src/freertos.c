#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "stm32f4xx_it.h"

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle, blinkyTaskHandle;
uint8_t rxbuf[3] = {0x00, 0x00};
uint8_t txbuf[3] = {0x0F | 0x80, 0x00};	// 0x0F is WHO_AM_I register, 0x80 read bit, should return 0b11010100 or 0xD4

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void vBlinkyTask(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

__attribute__((weak)) void vApplicationIdleHook( void ) {
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

	osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 1000);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	osThreadDef(blinkyTask, vBlinkyTask, osPriorityHigh, 4, 1000);
	blinkyTaskHandle = osThreadCreate(osThread(blinkyTask), NULL);
}

/* StartDefaultTask function which blinks LED2 every 1s */
void StartDefaultTask(void const * argument) {

  for(;;) {
  }
}

/* vBlinkyTask function which blinks LED1 every 10s and outputs a string to UART */
void vBlinkyTask(void const * argument) {
	for(;;) {
		HAL_GPIO_TogglePin(LED_GREEN_Port, LED_GREEN_Pin);
		if(HAL_GPIO_ReadPin(LED_GREEN_Port, LED_GREEN_Pin) == GPIO_PIN_RESET) {
			printf("Blink!\r\n");
		}
		osDelay(5000);
	}
}
