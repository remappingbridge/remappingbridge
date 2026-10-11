#ifndef BLU2USB_MODIFIER_SHORTCUTS_H
#define BLU2USB_MODIFIER_SHORTCUTS_H

#include <stdbool.h>
#include <stdint.h>
#include "blu2usb/domain/hid.h"
#include "blu2usb/domain/profile.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/profiles/profiles.h"

#define BLU2USB_MOD05_QUEUE_CAPACITY 8u

typedef enum {
    BLU2USB_MOD05_USB_IDLE = 0,
    BLU2USB_MOD05_USB_WAIT_DOWN,
    BLU2USB_MOD05_USB_NEED_UP,
    BLU2USB_MOD05_USB_WAIT_UP
} blu2usb_mod05_usb_phase_t;

typedef struct {
    bool enabled;
    uint8_t modifier_source; /* 0..4, or SOURCE_COUNT when not configured */
    uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT];
    bool modifier_down;
    bool secondary_used;
    uint8_t consumed_buttons; /* physical source bits whose down was consumed */
    uint8_t pending[BLU2USB_MOD05_QUEUE_CAPACITY];
    uint8_t queue_head, queue_size;
    blu2usb_mod05_usb_phase_t usb_phase;
    uint8_t running_action;
} blu2usb_modifier_shortcuts_t;

/* MOD-05 executes only Escape (ID 7) and Copy (ID 1).
 * The rest of the catalog is preserved but not falsely emitted. */
bool blu2usb_modifier_shortcuts_action_supported(uint8_t shortcut_id);
void blu2usb_modifier_shortcuts_init(blu2usb_modifier_shortcuts_t *state);
void blu2usb_modifier_shortcuts_cancel(blu2usb_modifier_shortcuts_t *state,
                                      blu2usb_hid_aggregator_t *aggregator);
void blu2usb_modifier_shortcuts_configure(blu2usb_modifier_shortcuts_t *state,
    const blu2usb_mouse_profile_config_t *profile,
    const uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT],
    blu2usb_hid_aggregator_t *aggregator);
void blu2usb_modifier_shortcuts_assignments(blu2usb_modifier_shortcuts_t *state,
    const uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT]);

/* Returns TRUE if input must continue to the existing remap pipeline. */
bool blu2usb_modifier_shortcuts_process(blu2usb_modifier_shortcuts_t *state,
    const blu2usb_canonical_mouse_event_t *event);

/* Two-phase USB report handshake. Call before keyboard report service, then
 * acknowledge ONLY when the report was sent (or matches accepted USB state).
 * A key-down and key-up can never be collapsed in one service tick. */
void blu2usb_modifier_shortcuts_prepare_usb(blu2usb_modifier_shortcuts_t *state,
    blu2usb_hid_aggregator_t *aggregator);
void blu2usb_modifier_shortcuts_ack_usb(blu2usb_modifier_shortcuts_t *state,
    bool report_accepted);

#endif
