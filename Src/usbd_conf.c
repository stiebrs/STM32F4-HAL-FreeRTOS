#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"

PCD_HandleTypeDef hpcd_USB_OTG_HS;
void _Error_Handler(char * file, int line);

void SystemClock_Config(void);

/*******************************************************************************
 LL Driver Callbacks (PCD -> USB Device Library)
 *******************************************************************************/
/* MSP Init */

void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle) {
	GPIO_InitTypeDef GPIO_InitStruct;
	if (pcdHandle->Instance == USB_OTG_HS) {

		/**USB_OTG_HS GPIO Configuration
		 PB12		 ------> USB_OTG_HS_ID
		 PB13		 ------> USB_OTG_HS_VBUS
		 PB14		 ------> USB_OTG_HS_DM
		 PB15		 ------> USB_OTG_HS_DP
		 */
		GPIO_InitStruct.Pin = OTG_HS_ID_Pin | OTG_HS_DM_Pin | OTG_HS_DP_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF12_OTG_HS_FS;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = VBUS_HS_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(VBUS_HS_GPIO_Port, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_USB_OTG_HS_CLK_ENABLE()
		;

		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
	}
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle) {
	if (pcdHandle->Instance == USB_OTG_HS) {
		/* Peripheral clock disable */
		__HAL_RCC_USB_OTG_HS_CLK_DISABLE();

		/**USB_OTG_HS GPIO Configuration
		 PB12		 ------> USB_OTG_HS_ID
		 PB13		 ------> USB_OTG_HS_VBUS
		 PB14		 ------> USB_OTG_HS_DM
		 PB15		 ------> USB_OTG_HS_DP
		 */
		HAL_GPIO_DeInit(GPIOB,
				OTG_HS_ID_Pin | VBUS_HS_Pin | OTG_HS_DM_Pin | OTG_HS_DP_Pin);

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(OTG_HS_IRQn);

	}
}

/**
 * @brief	Setup stage callback
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd) {
	USBD_LL_SetupStage((USBD_HandleTypeDef*) hpcd->pData,
			(uint8_t *) hpcd->Setup);
}

/**
 * @brief	Data Out stage callback.
 * @param	hpcd: PCD handle
 * @param	epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	USBD_LL_DataOutStage((USBD_HandleTypeDef*) hpcd->pData, epnum,
			hpcd->OUT_ep[epnum].xfer_buff);
}

/**
 * @brief	Data In stage callback.
 * @param	hpcd: PCD handle
 * @param	epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	USBD_LL_DataInStage((USBD_HandleTypeDef*) hpcd->pData, epnum,
			hpcd->IN_ep[epnum].xfer_buff);
}

/**
 * @brief	SOF callback.
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd) {
	USBD_LL_SOF((USBD_HandleTypeDef*) hpcd->pData);
}

/**
 * @brief	Reset callback.
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd) {
	USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

	/* Set USB current speed. */
	switch (hpcd->Init.speed) {
	case PCD_SPEED_HIGH:
		speed = USBD_SPEED_HIGH;
		break;
	case PCD_SPEED_FULL:
		speed = USBD_SPEED_FULL;
		break;

	default:
		speed = USBD_SPEED_FULL;
		break;
	}
	USBD_LL_SetSpeed((USBD_HandleTypeDef*) hpcd->pData, speed);

	/* Reset Device. */
	USBD_LL_Reset((USBD_HandleTypeDef*) hpcd->pData);
}

/**
 * @brief	Suspend callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd) {
	/* Inform USB library that core enters in suspend Mode. */
	USBD_LL_Suspend((USBD_HandleTypeDef*) hpcd->pData);
	__HAL_PCD_GATE_PHYCLOCK(hpcd);
	/* Enter in STOP mode. */
	if (hpcd->Init.low_power_enable) {
		/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
		SCB->SCR |= (uint32_t) ((uint32_t) (SCB_SCR_SLEEPDEEP_Msk
				| SCB_SCR_SLEEPONEXIT_Msk));
	}
}

/**
 * @brief	Resume callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd) {
	USBD_LL_Resume((USBD_HandleTypeDef*) hpcd->pData);
}

/**
 * @brief	ISOOUTIncomplete callback.
 * @param	hpcd: PCD handle
 * @param	epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*) hpcd->pData, epnum);
}

/**
 * @brief	ISOINIncomplete callback.
 * @param	hpcd: PCD handle
 * @param	epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*) hpcd->pData, epnum);
}

/**
 * @brief	Connect callback.
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
	USBD_LL_DevConnected((USBD_HandleTypeDef*) hpcd->pData);
}

/**
 * @brief	Disconnect callback.
 * @param	hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd) {
	USBD_LL_DevDisconnected((USBD_HandleTypeDef*) hpcd->pData);
}

/*******************************************************************************
 LL Driver Interface (USB Device Library --> PCD)
 *******************************************************************************/

/**
 * @brief	Initializes the low level portion of the device driver.
 * @param	pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev) {
	/* Init USB Ip. */
	if (pdev->id == DEVICE_HS) {
		/* Link the driver to the stack. */
		hpcd_USB_OTG_HS.pData = pdev;
		pdev->pData = &hpcd_USB_OTG_HS;

		hpcd_USB_OTG_HS.Instance = USB_OTG_HS;
		hpcd_USB_OTG_HS.Init.dev_endpoints = 6;
		hpcd_USB_OTG_HS.Init.speed = PCD_SPEED_FULL;
		hpcd_USB_OTG_HS.Init.dma_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.ep0_mps = DEP0CTL_MPS_64;
		hpcd_USB_OTG_HS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
		hpcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.lpm_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.vbus_sensing_enable = ENABLE;
		hpcd_USB_OTG_HS.Init.use_dedicated_ep1 = DISABLE;
		hpcd_USB_OTG_HS.Init.use_external_vbus = DISABLE;
		if (HAL_PCD_Init(&hpcd_USB_OTG_HS) != HAL_OK) {
			_Error_Handler(__FILE__, __LINE__);
		}

		HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
		HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x80);
		HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x174);
	}
	return USBD_OK;
}

/**
 * @brief	De-Initializes the low level portion of the device driver.
 * @param	pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_DeInit(pdev->pData);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Starts the low level portion of the device driver.
 * @param	pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_Start(pdev->pData);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Stops the low level portion of the device driver.
 * @param	pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_Stop(pdev->pData);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Opens an endpoint of the low level driver.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @param	ep_type: Endpoint type
 * @param	ep_mps: Endpoint max packet size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr,
		uint8_t ep_type, uint16_t ep_mps) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Closes an endpoint of the low level driver.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Flushes an endpoint of the Low Level Driver.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Sets a Stall condition on an endpoint of the Low Level Driver.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Clears a Stall condition on an endpoint of the Low Level Driver.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev,
		uint8_t ep_addr) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Returns Stall condition.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval Stall (1: Yes, 0: No)
 */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
	PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

	if ((ep_addr & 0x80) == 0x80) {
		return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
	} else {
		return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
	}
}

/**
 * @brief	Assigns a USB address to the device.
 * @param	pdev: Device handle
 * @param	dev_addr: Device address
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev,
		uint8_t dev_addr) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Transmits data over an endpoint.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @param	pbuf: Pointer to data to be sent
 * @param	size: Data size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr,
		uint8_t *pbuf, uint16_t size) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Prepares an endpoint for reception.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @param	pbuf: Pointer to data to be received
 * @param	size: Data size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
		uint8_t ep_addr, uint8_t *pbuf, uint16_t size) {
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

	switch (hal_status) {
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
 * @brief	Returns the last transfered packet size.
 * @param	pdev: Device handle
 * @param	ep_addr: Endpoint number
 * @retval Recived Data Size
 */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr) {
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

#if (USBD_LPM_ENABLED == 1)
/**
 * @brief	Send LPM message to user layer
 * @param	hpcd: PCD handle
 * @param	msg: LPM message
 * @retval None
 */
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg) {
	switch (msg) {
		case PCD_LPM_L0_ACTIVE:
		if (hpcd->Init.low_power_enable) {
			SystemClock_Config();

			/* Reset SLEEPDEEP bit of Cortex System Control Register. */
			SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}
		__HAL_PCD_UNGATE_PHYCLOCK(hpcd);
		USBD_LL_Resume(hpcd->pData);
		break;

		case PCD_LPM_L1_ACTIVE:
		__HAL_PCD_GATE_PHYCLOCK(hpcd);
		USBD_LL_Suspend(hpcd->pData);

		/* Enter in STOP mode. */
		if (hpcd->Init.low_power_enable) {
			/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
			SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}
		break;
	}
}
#endif /* (USBD_LPM_ENABLED == 1) */

/**
 * @brief	Delays routine for the USB Device Library.
 * @param	Delay: Delay in ms
 * @retval None
 */
void USBD_LL_Delay(uint32_t Delay) {
	HAL_Delay(Delay);
}
