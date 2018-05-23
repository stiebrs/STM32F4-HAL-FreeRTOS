/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "spi.h"

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle, blinkyTaskHandle, gyroTaskHandle;

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void vBlinkyTask(void const * argument);
void vGyroTesterTask(void const * argument);

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

	osThreadDef(blinkyTask, vBlinkyTask, osPriorityLow, 0, 1000);
	blinkyTaskHandle = osThreadCreate(osThread(blinkyTask), NULL);

	osThreadDef(gyroTask, vGyroTesterTask, osPriorityHigh, 0, 1000);
	gyroTaskHandle = osThreadCreate(osThread(gyroTask), NULL);
}

/* StartDefaultTask function which blinks LED2 every 1s */
void StartDefaultTask(void const * argument) {

  for(;;) {
	osDelay(500);
	HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14);

  }
}

/* vBlinkyTask function which blinks LED1 every 2s and outputs a string to UART */
void vBlinkyTask(void const * argument) {
	for(;;) {
		HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
		if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_13) == GPIO_PIN_RESET) {
			printf("Blink!\r\n");
		}
		osDelay(1000);
	}
}

void vGyroTesterTask(void const * argument) {
	HAL_StatusTypeDef response = HAL_ERROR; // default to error
	uint8_t txbuf[3] = {0x0F | 0x80, 0x00};	// 0x0F is WHO_AM_I register, 0x80 read bit, should return 0b11010100 or 0xD4
	uint8_t rxbuf[3] = {0x00, 0x00};

	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_RESET);
	response = HAL_SPI_TransmitReceive(&hspi5, txbuf, rxbuf, 2, 1000);
	HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_SET);
	if (response == HAL_OK) {
		printf("Sent: %02x %02x Got: %02x %02x\r\n", txbuf[0], txbuf[1], rxbuf[0], rxbuf[1]);
	} else {
		printf("Got error response as %d\r\n", response);
	}
	osThreadTerminate(gyroTaskHandle);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
