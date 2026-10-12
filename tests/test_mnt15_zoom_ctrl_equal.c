/* MNT-15: one generic Ctrl+Equal zoom-in chord, never an implicit Shift. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"

static bool pressed(const blu2usb_hid_output_state_t *state, uint8_t key)
{
    return (state->key_bitmap[key >> 3u] & (1u << (key & 7u))) != 0u;
}

static void check_shortcut(uint8_t action, uint8_t usage, uint8_t modifiers)
{
    blu2usb_keyboard_shortcut_t spec = {0};
    assert(blu2usb_keyboard_shortcut_lookup(action, &spec));
    assert(spec.key_usage == usage && spec.modifier_mask == modifiers);
    assert(blu2usb_modifier_shortcuts_action_supported(action));

    blu2usb_modifier_shortcuts_t shortcuts;
    blu2usb_hid_aggregator_t aggregator;
    blu2usb_modifier_shortcuts_init(&shortcuts);
    blu2usb_hid_aggregator_init(&aggregator);
    shortcuts.pending[0] = action;
    shortcuts.queue_size = 1u;

    blu2usb_modifier_shortcuts_prepare_usb(&shortcuts, &aggregator);
    assert(shortcuts.running_action == action);
    assert(shortcuts.usb_phase == BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_hid_output_state_t state;
    blu2usb_hid_aggregator_snapshot(&aggregator, &state);
    assert(pressed(&state, usage));
    assert(state.modifiers == modifiers);
    if (action == 26u)
        assert((state.modifiers & (1u << BLU2USB_MOD_LEFT_SHIFT)) == 0u);

    /* A busy USB endpoint must not silently lift or change the held chord. */
    blu2usb_modifier_shortcuts_ack_usb(&shortcuts, false);
    blu2usb_modifier_shortcuts_prepare_usb(&shortcuts, &aggregator);
    blu2usb_hid_aggregator_snapshot(&aggregator, &state);
    assert(pressed(&state, usage) && state.modifiers == modifiers);

    blu2usb_modifier_shortcuts_ack_usb(&shortcuts, true);
    assert(shortcuts.usb_phase == BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_prepare_usb(&shortcuts, &aggregator);
    blu2usb_hid_aggregator_snapshot(&aggregator, &state);
    assert(!pressed(&state, usage) && state.modifiers == 0u);
    blu2usb_modifier_shortcuts_ack_usb(&shortcuts, false);
    assert(shortcuts.usb_phase == BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&shortcuts, true);
    assert(shortcuts.usb_phase == BLU2USB_MOD05_USB_IDLE);
    assert(shortcuts.queue_size == 0u && shortcuts.running_action == 0u);

    /* Reconfiguration/cancellation cannot leave a modifier or key held. */
    blu2usb_modifier_shortcuts_cancel(&shortcuts, &aggregator);
    blu2usb_hid_aggregator_snapshot(&aggregator, &state);
    assert(!pressed(&state, usage) && state.modifiers == 0u);
}

int main(void)
{
    for (unsigned repeat = 0; repeat < 4u; ++repeat)
        check_shortcut(26u, 0x2eu, 0x01u); /* Zoom In: Ctrl+= only */

    check_shortcut(27u, 0x2du, 0x01u); /* Zoom Out remains Ctrl+- */
    check_shortcut(28u, 0x27u, 0x01u); /* Reset remains Ctrl+0 */
    check_shortcut(6u, 0x1du, 0x03u);  /* Redo still needs Ctrl+Shift */
    check_shortcut(15u, 0x46u, 0x0au);/* Area screenshot still Meta+Shift */
    return 0;
}
