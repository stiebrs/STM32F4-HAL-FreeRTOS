#include "tim.h"

TIM_HandleTypeDef htim1;

/* TIM1 init function */
void MX_TIM1_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 0;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {

	if (tim_baseHandle->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {

	if (tim_baseHandle->Instance == TIM1) {
		/* Peripheral clock disable */
		__HAL_RCC_TIM1_CLK_DISABLE();
	}
}
