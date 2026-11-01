#pragma once
#include "contacts.h"
void trackpad_usb_init(void);
void trackpad_usb_task(const tp_frame *latest, bool *pending);
