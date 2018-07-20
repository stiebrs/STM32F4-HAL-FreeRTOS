#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void) {
  
	/* Init Device Library, add supported class and start the library. */
	USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS);

	USBD_RegisterClass(&hUsbDeviceHS, &USBD_HID);

	USBD_Start(&hUsbDeviceHS);
}
