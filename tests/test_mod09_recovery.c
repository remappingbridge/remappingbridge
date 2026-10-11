/* MOD-09: interruption, busy USB, bounded overflow, neutral ownership.
 * All assertions run in the host CI; physical tests remain operator-owned. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "blu2usb/domain/hid.h"
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/movement/movement.h"

static blu2usb_canonical_mouse_event_t mouse_button(
    blu2usb_mouse_button_t button, bool pressed)
{
    blu2usb_canonical_mouse_event_t event = {0};
    event.source = blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE, 1u);
    event.type = BLU2USB_MOUSE_EVENT_BUTTON;
    event.data.button.button = button;
    event.data.button.pressed = pressed;
    return event;
}

static blu2usb_canonical_mouse_event_t wheel(int16_t vertical)
{
    blu2usb_canonical_mouse_event_t event = mouse_button(
        BLU2USB_MOUSE_BUTTON_LEFT, false);
    event.type = BLU2USB_MOUSE_EVENT_WHEEL;
    event.data.wheel.vertical = vertical;
    return event;
}

static bool send(blu2usb_modifier_shortcuts_t *shortcuts,
                 blu2usb_canonical_mouse_event_t event)
{
    blu2usb_canonical_mouse_event_t forwarded;
    return blu2usb_modifier_shortcuts_process_event(
        shortcuts, &event, blu2usb_movement_defaults(), 100u, &forwarded);
}

static void setup(blu2usb_modifier_shortcuts_t *shortcuts,
                  blu2usb_hid_aggregator_t *aggregator)
{
    blu2usb_mouse_profile_config_t profile = {0};
    profile.kind = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    profile.targets[BLU2USB_MOUSE_SOURCE_LEFT] = BLU2USB_MOUSE_TARGET_MODIFIER;
    uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT] = {0};
    assignments[0] = 7u; /* solo release */
    assignments[3] = 19u; /* right -> Consumer PLAY */
    assignments[6] = 22u; /* wheel up -> Consumer VOLUME UP */
    assignments[8] = 1u; /* left gesture -> Keyboard */
    blu2usb_hid_aggregator_init(aggregator);
    blu2usb_modifier_shortcuts_init(shortcuts);
    blu2usb_modifier_shortcuts_configure(
        shortcuts, &profile, assignments, aggregator);
    assert(shortcuts->enabled);
}

static void external_key(blu2usb_hid_aggregator_t *aggregator, bool down)
{
    blu2usb_canonical_keyboard_event_t event = {0};
    event.source = blu2usb_hid_source_make(BLU2USB_HID_SOURCE_KEYBOARD, 1u);
    event.type = BLU2USB_KEYBOARD_EVENT_KEY;
    event.data.key.key = BLU2USB_KEY_A;
    event.data.key.pressed = down;
    assert(blu2usb_hid_aggregator_apply_keyboard(aggregator, &event));
}

static void check_ble_disconnect_and_lock(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t a;
    setup(&s, &a);
    external_key(&a, true);
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, true)));
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_RIGHT, true)));
    assert(s.queue_size == 1u);
    blu2usb_modifier_shortcuts_prepare_usb(&s, &a);
    assert(s.running_consumer && s.consumer_usage == 0x00cdu);
    blu2usb_modifier_shortcuts_ack_usb(&s, false); /* USB busy */
    blu2usb_modifier_shortcuts_cancel(&s, &a); /* BLE disconnect */
    assert(s.queue_size == 0u && !s.modifier_down && !s.running_action);
    assert(s.usb_phase == BLU2USB_MOD05_USB_IDLE);
    assert(s.consumer_usage == 0u && !s.area_drag_down);
    assert(a.key_refs[BLU2USB_KEY_A] == 1u); /* other source stays pressed */
    blu2usb_modifier_shortcuts_cancel(&s, &a); /* lock/repeated cancel */
    assert(a.key_refs[BLU2USB_KEY_A] == 1u);
    external_key(&a, false);
    assert(a.key_refs[BLU2USB_KEY_A] == 0u);
    /* Reconfiguration cannot resurrect the interrupted queue. */
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, true)));
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, false)));
    assert(s.queue_size == 1u); /* NEW solo release is allowed */
}

static void check_keyboard_cancel_and_recovery(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t a;
    setup(&s, &a);
    external_key(&a, true);
    s.pending[0] = 1u;
    s.queue_size = 1u;
    blu2usb_modifier_shortcuts_prepare_usb(&s, &a);
    assert(s.usb_phase == BLU2USB_MOD05_USB_WAIT_DOWN);
    assert(a.key_refs[0x06u] == 1u); /* Ctrl+C held by synthetic */
    assert(a.modifier_refs[BLU2USB_MOD_LEFT_CTRL] == 1u);
    blu2usb_modifier_shortcuts_ack_usb(&s, true);
    blu2usb_modifier_shortcuts_cancel(&s, &a); /* profile apply/lock */
    assert(a.key_refs[0x06u] == 0u);
    assert(a.modifier_refs[BLU2USB_MOD_LEFT_CTRL] == 0u);
    assert(a.key_refs[BLU2USB_KEY_A] == 1u);
    assert(s.usb_phase == BLU2USB_MOD05_USB_IDLE);
    external_key(&a, false);
}

static void check_bounded_wheel_and_unsupported_move(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t a;
    setup(&s, &a);
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, true)));
    assert(!send(&s, wheel(40))); /* never allocate unbounded events */
    assert(s.queue_size == BLU2USB_MOD05_QUEUE_CAPACITY);
    assert(s.secondary_used);
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, false)));
    assert(s.queue_size == BLU2USB_MOD05_QUEUE_CAPACITY);
    blu2usb_modifier_shortcuts_cancel(&s, &a);
    assert(s.queue_size == 0u);
    /* A corrupted future ID in the MOVE range must not swallow real
     * pointer movement just because it is nonzero. */
    s.assignments[8] = 31u;
    s.assignments[9] = 0u;
    s.assignments[10] = 0u;
    s.assignments[11] = 0u;
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, true)));
    blu2usb_canonical_mouse_event_t mv = mouse_button(
        BLU2USB_MOUSE_BUTTON_LEFT, false);
    mv.type = BLU2USB_MOUSE_EVENT_MOVE;
    mv.data.move.dx = 20;
    assert(send(&s, mv));
    assert(!s.gesture_latched);
    assert(!send(&s, mouse_button(BLU2USB_MOUSE_BUTTON_LEFT, false)));
}

int main(void)
{
    check_ble_disconnect_and_lock();
    check_keyboard_cancel_and_recovery();
    check_bounded_wheel_and_unsupported_move();
    return 0;
}
