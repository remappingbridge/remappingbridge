#include "blu2usb/modifier_shortcuts/consumer_catalog.h"
#include <stddef.h>
/* Semantic IDs and USB-HID Usage Tables Consumer page 0x0C.
 * Host backlight mapping, especially brightness, is not guaranteed. */
static const uint16_t k_usage[31] = {
    [19] = 0x00CDu, /* Play/Pause */
    [20] = 0x00B5u, /* Scan Next Track */
    [21] = 0x00B6u, /* Scan Previous Track */
    [22] = 0x00E9u, /* Volume Increment */
    [23] = 0x00EAu, /* Volume Decrement */
    [24] = 0x00E2u, /* Mute */
    [29] = 0x006Fu, /* Brightness Increment */
    [30] = 0x0070u, /* Brightness Decrement */
};
bool blu2usb_consumer_shortcut_lookup(uint8_t action_id, uint16_t *usage)
{
    if (!usage || action_id == 0 || action_id >= 31 || !k_usage[action_id])
        return false;
    *usage = k_usage[action_id];
    return true;
}
