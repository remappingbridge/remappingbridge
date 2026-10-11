#ifndef BLU2USB_MODIFIER_SHORTCUTS_H
#define BLU2USB_MODIFIER_SHORTCUTS_H

#include <stdbool.h>
#include <stdint.h>
#include "blu2usb/domain/hid.h"
#include "blu2usb/domain/profile.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/movement/movement.h"
#include "blu2usb/profiles/profiles.h"

#define BLU2USB_MOD05_QUEUE_CAPACITY 8u
/* MOD-06 provisional physical calibration (raw counts before DPI gain). */
#define BLU2USB_MOD06_MOVE_THRESHOLD 24
#define BLU2USB_MOD06_DOMINANCE_RATIO 2
#define BLU2USB_MOD06_QUIET_MS 160u

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
    uint8_t physical_buttons; /* edge detection for genuine press vs held repeats */
    uint8_t tap_motion; /* raw-count jitter budget for solo release */
    bool gesture_latched;
    bool motion_seen;
    int32_t gesture_x;
    int32_t gesture_y;
    uint32_t motion_last_ms;
    uint8_t pending[BLU2USB_MOD05_QUEUE_CAPACITY];
    uint8_t queue_head, queue_size;
    blu2usb_mod05_usb_phase_t usb_phase;
    uint8_t running_action;
} blu2usb_modifier_shortcuts_t;

/* MOD-07: semantic IDs 1..18 and 25..28 are executable with Keyboard HID;
 * Consumer Media/Brightness IDs 19..24 and 29..30 remain MOD-08 only. */
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

/* MOD-06 production path; emits a filtered canonical event when forwarding.
 * Wheel detents map to one queued action each (when assigned and executable);
 * pan is never interpreted as vertical wheel. Captured MOVE does not reach
 * Mouse or Movement accumulator, and uses the shared pure rotation helper. */
bool blu2usb_modifier_shortcuts_process_event(
    blu2usb_modifier_shortcuts_t *state,
    const blu2usb_canonical_mouse_event_t *event,
    blu2usb_movement_settings_t settings,
    uint32_t now_ms,
    blu2usb_canonical_mouse_event_t *forwarded);
void blu2usb_modifier_shortcuts_tick(blu2usb_modifier_shortcuts_t *state,
                                     uint32_t now_ms);

/* Two-phase USB report handshake. Call before keyboard report service, then
 * acknowledge ONLY when the report was sent (or matches accepted USB state).
 * A key-down and key-up can never be collapsed in one service tick. */
void blu2usb_modifier_shortcuts_prepare_usb(blu2usb_modifier_shortcuts_t *state,
    blu2usb_hid_aggregator_t *aggregator);
void blu2usb_modifier_shortcuts_ack_usb(blu2usb_modifier_shortcuts_t *state,
    bool report_accepted);

#endif
