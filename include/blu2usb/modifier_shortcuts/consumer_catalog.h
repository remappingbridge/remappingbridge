#ifndef BLU2USB_MODIFIER_SHORTCUTS_CONSUMER_CATALOG_H
#define BLU2USB_MODIFIER_SHORTCUTS_CONSUMER_CATALOG_H
#include <stdbool.h>
#include <stdint.h>
/* HID Usage Page 0x0C Consumer; 16-bit usage, not Keyboard page 0x07. */
bool blu2usb_consumer_shortcut_lookup(uint8_t action_id, uint16_t *usage);
#endif
