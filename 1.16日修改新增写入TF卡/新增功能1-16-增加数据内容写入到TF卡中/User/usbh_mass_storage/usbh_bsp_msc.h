

#ifndef _USBH_BSP_USR_H
#define _USBH_BSP_USR_H

#include "usb_conf.h"
#include "usb_bsp.h"
#include "usb_hcd_int.h"
#include "usbh_core.h"

#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "usbh_msc_core.h"

#include "usbh_usr.h"

//void usbd_OpenMassStorage(void);
//void usbd_CloseMassStorage(void);

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;

#endif
