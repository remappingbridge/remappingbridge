#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"

#include <string.h>

#define BLU2USB_KEY_C_USAGE 0x06u

static blu2usb_hid_source_t shortcut_source(void)
{
    return blu2usb_hid_source_make(BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT, 1u);
}

static int physical_source(blu2usb_mouse_button_t button)
{
    switch (button) {
    case BLU2USB_MOUSE_BUTTON_LEFT: return BLU2USB_MOUSE_SOURCE_LEFT;
    case BLU2USB_MOUSE_BUTTON_RIGHT: return BLU2USB_MOUSE_SOURCE_RIGHT;
    case BLU2USB_MOUSE_BUTTON_MIDDLE: return BLU2USB_MOUSE_SOURCE_MIDDLE;
    case BLU2USB_MOUSE_BUTTON_FORWARD: return BLU2USB_MOUSE_SOURCE_FORWARD;
    case BLU2USB_MOUSE_BUTTON_BACK: return BLU2USB_MOUSE_SOURCE_BACKWARD;
    default: return -1;
    }
}

static unsigned button_combo(int source)
{
    switch (source) {
    case BLU2USB_MOUSE_SOURCE_MIDDLE: return 1u;
    case BLU2USB_MOUSE_SOURCE_LEFT: return 2u;
    case BLU2USB_MOUSE_SOURCE_RIGHT: return 3u;
    case BLU2USB_MOUSE_SOURCE_FORWARD: return 4u;
    case BLU2USB_MOUSE_SOURCE_BACKWARD: return 5u;
    default: return BLU2USB_SHORTCUT_SLOT_COUNT;
    }
}

bool blu2usb_modifier_shortcuts_action_supported(uint8_t id)
{
    return id == 1u || id == 7u; /* COPY = Ctrl+C, ESCAPE = HID 0x29 */
}

void blu2usb_modifier_shortcuts_init(blu2usb_modifier_shortcuts_t *state)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->modifier_source = BLU2USB_MOUSE_SOURCE_COUNT;
}

void blu2usb_modifier_shortcuts_cancel(blu2usb_modifier_shortcuts_t *state,
                                      blu2usb_hid_aggregator_t *aggregator)
{
    if (state == NULL) return;
    if (aggregator != NULL)
        (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
    state->modifier_down = false;
    state->secondary_used = false;
    state->consumed_buttons = 0u;
    state->physical_buttons = 0u;
    state->tap_motion = 0u;
    state->queue_head = state->queue_size = 0u;
    state->usb_phase = BLU2USB_MOD05_USB_IDLE;
    state->running_action = 0u;
}

void blu2usb_modifier_shortcuts_configure(blu2usb_modifier_shortcuts_t *state,
    const blu2usb_mouse_profile_config_t *profile,
    const uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT],
    blu2usb_hid_aggregator_t *aggregator)
{
    if (state == NULL) return;
    blu2usb_modifier_shortcuts_cancel(state, aggregator);
    state->enabled = false;
    state->modifier_source = BLU2USB_MOUSE_SOURCE_COUNT;
    if (assignments != NULL)
        memcpy(state->assignments, assignments, sizeof(state->assignments));
    else
        memset(state->assignments, 0, sizeof(state->assignments));
    if (profile == NULL || profile->kind != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP)
        return;
    unsigned seen = 0u;
    for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
        if (profile->targets[i] == BLU2USB_MOUSE_TARGET_MODIFIER) {
            state->modifier_source = (uint8_t)i;
            ++seen;
        }
    }
    state->enabled = seen == 1u;
    if (!state->enabled)
        state->modifier_source = BLU2USB_MOUSE_SOURCE_COUNT;
}

void blu2usb_modifier_shortcuts_assignments(blu2usb_modifier_shortcuts_t *state,
    const uint8_t assignments[BLU2USB_SHORTCUT_SLOT_COUNT])
{
    if (state != NULL && assignments != NULL)
        memcpy(state->assignments, assignments, sizeof(state->assignments));
}

static void queue_action(blu2usb_modifier_shortcuts_t *state, uint8_t id)
{
    if (!blu2usb_modifier_shortcuts_action_supported(id) ||
        state->queue_size == BLU2USB_MOD05_QUEUE_CAPACITY)
        return; /* bounded: never block BLE/HAT or latch an unowned key */
    const unsigned tail = (state->queue_head + state->queue_size) %
                          BLU2USB_MOD05_QUEUE_CAPACITY;
    state->pending[tail] = id;
    ++state->queue_size;
}

bool blu2usb_modifier_shortcuts_process(blu2usb_modifier_shortcuts_t *state,
    const blu2usb_canonical_mouse_event_t *event)
{
    if (state == NULL || event == NULL || !state->enabled) return true;
    if (event->type == BLU2USB_MOUSE_EVENT_BUTTON) {
        const int source = physical_source(event->data.button.button);
        if (source < 0) {
            if (state->modifier_down && event->data.button.pressed)
                state->secondary_used = true;
            return true;
        }
        const uint8_t bit = (uint8_t)(1u << (unsigned)source);
        const bool was_pressed = (state->physical_buttons & bit) != 0u;
        if (event->data.button.pressed)
            state->physical_buttons |= bit;
        else
            state->physical_buttons &= (uint8_t)~bit;
        if ((unsigned)source == state->modifier_source) {
            if (event->data.button.pressed) {
                if (!state->modifier_down) {
                    state->modifier_down = true;
                    state->tap_motion = 0u;
                    state->secondary_used =
                        (state->physical_buttons & (uint8_t)~bit) != 0u;
                }
            } else if (state->modifier_down) {
                if (!state->secondary_used)
                    queue_action(state, state->assignments[0]); /* MODIFIER RELEASED */
                state->modifier_down = false;
                state->secondary_used = false;
                state->tap_motion = 0u;
            }
            return false; /* Never leak physical modifier click */
        }
        if (!event->data.button.pressed) {
            if (state->consumed_buttons & bit) {
                state->consumed_buttons &= (uint8_t)~bit;
                return false; /* consumed down => consumed up, even after MOD up */
            }
            return true;
        }
        if (!state->modifier_down) return true;
        state->secondary_used = true;
        if (was_pressed) return (state->consumed_buttons & bit) == 0u;
        /* Never interpret a repeated HID pressed report as a fresh chord. */ /* Even an unassigned secondary aborts TAP */
        const unsigned combo = button_combo(source);
        if (combo < BLU2USB_SHORTCUT_SLOT_COUNT &&
            blu2usb_modifier_shortcuts_action_supported(state->assignments[combo])) {
            if ((state->consumed_buttons & bit) == 0u) {
                state->consumed_buttons |= bit;
                queue_action(state, state->assignments[combo]); /* edge, no repeat */
            }
            return false;
        }
        return true; /* Unassigned/unsupported buttons keep their normal remap */
    }
    if (state->modifier_down) {
        /* MOD-06 owns actual scroll/MOVE gestures; this gate never suppresses
         * or transforms relative deltas. A nonzero secondary event prevents
         * accidental tap release until the gesture classifier is introduced. */
        if (event->type == BLU2USB_MOUSE_EVENT_WHEEL &&
            (event->data.wheel.vertical || event->data.wheel.horizontal))
            state->secondary_used = true;
        if (event->type == BLU2USB_MOUSE_EVENT_MOVE &&
            (event->data.move.dx || event->data.move.dy)) {
            /* Raw relative counts, not scaled DPI. A small noise budget
             * avoids cancelling a clean tap on sensor jitter. This is not
             * the directional classifier (deferred to MOD-06). */
            const int32_t x = event->data.move.dx;
            const int32_t y = event->data.move.dy;
            const uint32_t movement =
                (uint32_t)(x < 0 ? -x : x) + (uint32_t)(y < 0 ? -y : y);
            const uint32_t total = (uint32_t)state->tap_motion + movement;
            state->tap_motion = (uint8_t)(total > 8u ? 8u : total);
            if (state->tap_motion >= 8u)
                state->secondary_used = true;
        }
    }
    return true;
}

static bool apply_key(blu2usb_hid_aggregator_t *agg,
                      uint8_t key, bool pressed)
{
    blu2usb_canonical_keyboard_event_t e = {0};
    e.source = shortcut_source();
    e.type = BLU2USB_KEYBOARD_EVENT_KEY;
    e.data.key.key = key;
    e.data.key.pressed = pressed;
    return blu2usb_hid_aggregator_apply_keyboard(agg, &e);
}

static bool apply_control(blu2usb_hid_aggregator_t *agg, bool pressed)
{
    blu2usb_canonical_keyboard_event_t e = {0};
    e.source = shortcut_source();
    e.type = BLU2USB_KEYBOARD_EVENT_MODIFIER;
    e.data.modifier.modifier = BLU2USB_MOD_LEFT_CTRL;
    e.data.modifier.pressed = pressed;
    return blu2usb_hid_aggregator_apply_keyboard(agg, &e);
}

void blu2usb_modifier_shortcuts_prepare_usb(blu2usb_modifier_shortcuts_t *state,
    blu2usb_hid_aggregator_t *aggregator)
{
    if (state == NULL || aggregator == NULL) return;
    if (state->usb_phase == BLU2USB_MOD05_USB_NEED_UP) {
        (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
        state->usb_phase = BLU2USB_MOD05_USB_WAIT_UP;
        return;
    }
    if (state->usb_phase != BLU2USB_MOD05_USB_IDLE || !state->queue_size)
        return;
    const uint8_t id = state->pending[state->queue_head];
    state->queue_head = (uint8_t)((state->queue_head + 1u) % BLU2USB_MOD05_QUEUE_CAPACITY);
    --state->queue_size;
    bool ok = true;
    if (id == 1u) ok = apply_control(aggregator, true) &&
                        apply_key(aggregator, BLU2USB_KEY_C_USAGE, true);
    else if (id == 7u) ok = apply_key(aggregator, BLU2USB_KEY_ESCAPE, true);
    else ok = false;
    if (!ok) {
        (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
        return;
    }
    state->running_action = id;
    state->usb_phase = BLU2USB_MOD05_USB_WAIT_DOWN;
}

void blu2usb_modifier_shortcuts_ack_usb(blu2usb_modifier_shortcuts_t *state,
    bool report_accepted)
{
    if (state == NULL || !report_accepted) return;
    if (state->usb_phase == BLU2USB_MOD05_USB_WAIT_DOWN)
        state->usb_phase = BLU2USB_MOD05_USB_NEED_UP;
    else if (state->usb_phase == BLU2USB_MOD05_USB_WAIT_UP) {
        state->usb_phase = BLU2USB_MOD05_USB_IDLE;
        state->running_action = 0u;
    }
}
