#ifndef BLU2USB_MODIFIER_SHORTCUTS_KEYBOARD_CATALOG_H
#define BLU2USB_MODIFIER_SHORTCUTS_KEYBOARD_CATALOG_H

#include <stdbool.h>
#include <stdint.h>

#define BLU2USB_KEYBOARD_SHORTCUT_MAX_ID 30u

/* USB HID Keyboard/Keypad usage IDs, not Unicode characters. The host
 * determines the actual action; MOD-07's first acceptance target is Linux. */
typedef struct {
    uint8_t key_usage;
    uint8_t modifier_mask; /* Standard 8-bit HID keyboard report modifiers. */
} blu2usb_keyboard_shortcut_t;

/* Only semantic IDs 1..18 and 25..28 are supported by the unchanged
 * boot-keyboard-compatible, two-interface USB descriptor. No Consumer
 * emulation or fake media/brightness usages. Invalid IDs return false. */
bool blu2usb_keyboard_shortcut_lookup(uint8_t action_id,
                                     blu2usb_keyboard_shortcut_t *out);

#endif
