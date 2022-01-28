#include "tim.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8;

// forward declarations
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle);

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

void MX_TIM8_Init(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	// All PWM channel outputs are based on this setup for single timer
	// If we look at stm32f4xx_hal_rcc_ex.h file section for __HAL_RCC_TIM8_CLK_ENABLE(), we see that it's running off APB2 clock which is running at 72MHz

	htim8.Instance = TIM8;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	// prescaler is 16-bit, dividing by 720 will give us 100kHz ticks
	htim8.Init.Prescaler = 720;
	// now we should be able to simply select the length we want in units of 0.01 milliseconds
	htim8.Init.Period = 100;
	htim8.Init.RepetitionCounter = 0;
	// This is not actual timer clock division, but used only for dead-time calculation
	// With 100kHz frequency one DTS (Deat Time Step) is 1/18MHz or 5.6ns
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	// Try to init
	if (HAL_TIM_Base_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}

	// Clock straight from internal clock.
	// This can be used to chain clocks to get super-long timer periods
	// much longer than maximum settings of a single timer would allow.
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
		Error_Handler();
	}

	// These calls are optional, since peripheral should be in this state by default
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}

	// Now set up actual PWM params
	// We want a simple PWM on TIM8 Channel 3
	// In Mode1 channel is active (of selected Polarity) as long as TIMx_CNT<TIMx_CCR1
	// In Mode2 channel is inactive (of selected Polarity) as long as TIMx_CNT<TIMx_CCR1
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	// Pulse length in clock ticks should give us 65ms
	sConfigOC.Pulse = 65;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}

	// we'll settle for 500 us on 500 us off with 50 us dead time (both pulled the same way)
	// We want complimentary (CHx = a, CHxN = !a) PWM on TIM8 Channel 2
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	// Pulse length in clock ticks should give us 65ms
	sConfigOC.Pulse = 50;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNIdleState = TIM_OCIDLESTATE_SET;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}

	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	// In counts of DTS periods (in this case 400us), max value 0xFF
	// t_DTS with 72MHz APB clock and predivider 4 to 18MHz period is ~56 ns
	// From TIMx_BDTR reg description
	// DTG[7:5]=0xx => DT=DTG[7:0]x t_dtg with t_dtg = t_DTS = 56 ns
	// DTG[7:5]=10x => DT=(64+DTG[5:0])x t_dtg with t_dtg = 2x t_DTS = 112 ns
	// DTG[7:5]=110 => DT=(32+DTG[4:0])x t_dtg with t_dtg = 8x t_DTS = 448 ns
	// DTG[7:5]=111 => DT=(32+DTG[4:0])x t_dtg with t_dtg = 16x t_DTS = 896 ns
	// so max that we can get is (32+31)(0.896) = 56.448 us
	// let's try to get 50us
	// 50000 = (32+x) * 16 * 56
	// x = 50000/16/46 - 32 = 23.8036
	// with x = 24, DT should be (32+24) * 16 * 56ns = 50 176 ns
	sBreakDeadTimeConfig.DeadTime = (0b11100000 | 24);
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK) {
		Error_Handler();
	}

	// configure pins
	HAL_TIM_MspPostInit(&htim8);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {
	if (tim_baseHandle->Instance == TIM1) {
		__HAL_RCC_TIM1_CLK_ENABLE();
	} else if (tim_baseHandle->Instance == TIM8) {
		__HAL_RCC_TIM8_CLK_ENABLE();
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {
	if (tim_baseHandle->Instance == TIM1) {
		/* Peripheral clock disable */
		__HAL_RCC_TIM1_CLK_DISABLE();
	} else if (tim_baseHandle->Instance == TIM8) {
		__HAL_RCC_TIM8_CLK_DISABLE();
	}
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(timHandle->Instance==TIM8) {
		// enable port C clocks in case they are not enabled yet
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**TIM8 GPIO Configuration
		PC7     ------> TIM8_CH2
		PC8     ------> TIM8_CH3
		PB0		------> TIM8_CH2N
		*/

		// CH 3 / 2 / 1
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		// CH 2N
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		// CH 1N
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

void start_TIM8(void) {
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
}
