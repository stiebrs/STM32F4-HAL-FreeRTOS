#ifndef __usb_otg_H
#define __usb_otg_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"

extern void _Error_Handler(char *, int);

void MX_USB_OTG_HS_USB_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ usb_otg_H */
