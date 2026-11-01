#pragma once
#include "contacts.h"
void trackpad_usb_init(void);
void trackpad_usb_task(const tp_frame *latest, bool *pending);
#ifdef TRACKPAD_MACOS
void trackpad_usb_reset_delivery(void);
bool trackpad_usb_delivered(void);
#endif
