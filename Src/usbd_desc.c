#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define USBD_VID     					0x0483
#define USBD_LANGID_STRING     			0x409
#define USBD_MANUFACTURER_STRING     	"STMicroelectronics"
#define USBD_PID_HS     				0x5710
#define USBD_PRODUCT_STRING_HS     		"STM32 Human interface"
#define USBD_SERIALNUMBER_STRING_HS     "00000000001A"
#define USBD_CONFIGURATION_STRING_HS    "HID Config"
#define USBD_INTERFACE_STRING_HS     	"HID Interface"

#define USB_SIZ_BOS_DESC            	0x0C

uint8_t * USBD_HS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_HS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

#ifdef USB_SUPPORT_USER_STRING_DESC
uint8_t * USBD_HS_USRStringDesc(USBD_SpeedTypeDef speed, uint8_t idx, uint16_t *length);
#endif /* USB_SUPPORT_USER_STRING_DESC */

#if (USBD_LPM_ENABLED == 1)
uint8_t * USBD_HS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
#endif /* (USBD_LPM_ENABLED == 1) */

USBD_DescriptorsTypeDef HS_Desc = { USBD_HS_DeviceDescriptor, USBD_HS_LangIDStrDescriptor,
		USBD_HS_ManufacturerStrDescriptor, USBD_HS_ProductStrDescriptor, USBD_HS_SerialStrDescriptor,
		USBD_HS_ConfigStrDescriptor, USBD_HS_InterfaceStrDescriptor
#if (USBD_LPM_ENABLED == 1)
		, USBD_HS_USR_BOSDescriptor
#endif /* (USBD_LPM_ENABLED == 1) */
};

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/** USB standard device descriptor. */
__ALIGN_BEGIN uint8_t USBD_HS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = { 0x12, /*bLength */
USB_DESC_TYPE_DEVICE, /*bDescriptorType*/
#if (USBD_LPM_ENABLED == 1)
		0x01, /*bcdUSB *//* changed to USB version 2.01
		 in order to support LPM L1 suspend
		 resume test of USBCV3.0*/
#else
		0x00, /*bcdUSB */
#endif /* (USBD_LPM_ENABLED == 1) */

		0x02, 0x00, /*bDeviceClass*/
		0x00, /*bDeviceSubClass*/
		0x00, /*bDeviceProtocol*/
		USB_MAX_EP0_SIZE, /*bMaxPacketSize*/
		LOBYTE(USBD_VID), /*idVendor*/
		HIBYTE(USBD_VID), /*idVendor*/
		LOBYTE(USBD_PID_HS), /*idProduct*/
		HIBYTE(USBD_PID_HS), /*idProduct*/
		0x00, /*bcdDevice rel. 2.00*/
		0x02,
		USBD_IDX_MFC_STR, /*Index of manufacturer  string*/
		USBD_IDX_PRODUCT_STR, /*Index of product string*/
		USBD_IDX_SERIAL_STR, /*Index of serial number string*/
		USBD_MAX_NUM_CONFIGURATION /*bNumConfigurations*/
};

/** BOS descriptor. */
#if (USBD_LPM_ENABLED == 1)
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN uint8_t USBD_HS_BOSDesc[USB_SIZ_BOS_DESC] __ALIGN_END =
{
	0x5,
	USB_DESC_TYPE_BOS,
	0xC,
	0x0,
	0x1, /* 1 device capability */
	/* device capability */
	0x7,
	USB_DEVICE_CAPABITY_TYPE,
	0x2,
	0x2, /*LPM capability bit set */
	0x0,
	0x0,
	0x0
};
#endif /* (USBD_LPM_ENABLED == 1) */

/**
 * @}
 */

/** @defgroup USBD_DESC_Private_Variables USBD_DESC_Private_Variables
 * @brief Private variables.
 * @{
 */

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */

/** USB lang indentifier descriptor. */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
USB_LEN_LANGID_STR_DESC,
USB_DESC_TYPE_STRING, LOBYTE(USBD_LANGID_STRING), HIBYTE(USBD_LANGID_STRING) };

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

/**
 * @}
 */

/** @defgroup USBD_DESC_Private_Functions USBD_DESC_Private_Functions
 * @brief Private functions.
 * @{
 */

/**
 * @brief  Return the device descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	*length = sizeof(USBD_HS_DeviceDesc);
	return USBD_HS_DeviceDesc;
}

/**
 * @brief  Return the LangID string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	*length = sizeof(USBD_LangIDDesc);
	return USBD_LangIDDesc;
}

/**
 * @brief  Return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t * USBD_HS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	if (speed == 0) {
		USBD_GetString((uint8_t *) USBD_PRODUCT_STRING_HS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t *) USBD_PRODUCT_STRING_HS, USBD_StrDesc, length);
	}
	return USBD_StrDesc;
}

/**
 * @brief  Return the manufacturer string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	USBD_GetString((uint8_t *) USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

/**
 * @brief  Return the serial number string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	if (speed == USBD_SPEED_HIGH) {
		USBD_GetString((uint8_t *) USBD_SERIALNUMBER_STRING_HS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t *) USBD_SERIALNUMBER_STRING_HS, USBD_StrDesc, length);
	}
	return USBD_StrDesc;
}

/**
 * @brief  Return the configuration string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	if (speed == USBD_SPEED_HIGH) {
		USBD_GetString((uint8_t *) USBD_CONFIGURATION_STRING_HS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t *) USBD_CONFIGURATION_STRING_HS, USBD_StrDesc, length);
	}
	return USBD_StrDesc;
}

/**
 * @brief  Return the interface string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	if (speed == 0) {
		USBD_GetString((uint8_t *) USBD_INTERFACE_STRING_HS, USBD_StrDesc, length);
	} else {
		USBD_GetString((uint8_t *) USBD_INTERFACE_STRING_HS, USBD_StrDesc, length);
	}
	return USBD_StrDesc;
}

#if (USBD_LPM_ENABLED == 1)
/**
 * @brief  Return the BOS descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t * USBD_HS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = sizeof(USBD_HS_BOSDesc);
	return (uint8_t*)USBD_HS_BOSDesc;
}
#endif /* (USBD_LPM_ENABLED == 1) */
