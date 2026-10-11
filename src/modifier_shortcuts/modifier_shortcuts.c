#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/consumer_catalog.h"

#include <string.h>


static blu2usb_hid_source_t shortcut_source(void)
{
    return blu2usb_hid_source_make(BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT, 1u);
}
/* Separate mouse-only source so keyboard release never lifts a drag. */
static blu2usb_hid_source_t area_drag_source(void)
{
    return blu2usb_hid_source_make(BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT, 2u);
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
    blu2usb_keyboard_shortcut_t spec;
    uint16_t usage = 0u;
    return blu2usb_keyboard_shortcut_lookup(id, &spec) ||
           blu2usb_consumer_shortcut_lookup(id, &usage);
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
    if (aggregator != NULL) {
        (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
        (void)blu2usb_hid_aggregator_release_source(aggregator, area_drag_source());
    }
    state->modifier_down = false;
    state->secondary_used = false;
    state->consumed_buttons = 0u;
    state->physical_buttons = 0u;
    state->tap_motion = 0u;
    state->area_selection = false;
    state->area_trigger_ready = false;
    state->area_drag_down = false;
    state->area_trigger_source = BLU2USB_MOUSE_SOURCE_COUNT;
    state->gesture_latched = false;
    state->motion_seen = false;
    state->gesture_x = state->gesture_y = 0;
    state->motion_last_ms = 0u;
    state->queue_head = state->queue_size = 0u;
    state->usb_phase = BLU2USB_MOD05_USB_IDLE;
    state->running_action = 0u;
    state->running_consumer = false;
    state->consumer_usage = 0u;
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
                    state->gesture_latched = false;
                    state->motion_seen = false;
                    state->gesture_x = state->gesture_y = 0;
                    state->secondary_used =
                        (state->physical_buttons & (uint8_t)~bit) != 0u;
                }
            } else if (state->modifier_down) {
                if (!state->secondary_used)
                    queue_action(state, state->assignments[0]); /* MODIFIER RELEASED */
                state->modifier_down = false;
                state->secondary_used = false;
                state->tap_motion = 0u;
                state->area_selection = false;
                state->area_trigger_ready = false;
                state->gesture_latched = false;
                state->motion_seen = false;
                state->gesture_x = state->gesture_y = 0;
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
                if (state->assignments[combo] == 15u) {
                    /* The first combo starts Spectacle's rectangular overlay.
                     * Keep pointer movement live even when MOVE is assigned;
                     * the next distinct click of this physical button is
                     * mapped to a synthetic LEFT down/up for drag selection. */
                    state->area_selection = true;
                    state->area_trigger_source = (uint8_t)source;
                    state->area_trigger_ready = false;
                    state->area_drag_down = false;
                }
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

/* Saturating to a modest raw-count window prevents signed overflow and
 * means gains (H/V DPI) never influence gesture threshold or direction. */
static int32_t bounded_add(int32_t lhs, int64_t rhs)
{
    const int64_t v = (int64_t)lhs + rhs;
    return v > 1024 ? 1024 : v < -1024 ? -1024 : (int32_t)v;
}

static uint32_t magnitude(int64_t v)
{
    if (v < 0) v = -v; /* receives int32 deltas, so no INT64_MIN */
    return (uint32_t)v;
}

void blu2usb_modifier_shortcuts_tick(blu2usb_modifier_shortcuts_t *state,
                                     uint32_t now_ms)
{
    if (state == NULL || !state->motion_seen) return;
    if ((uint32_t)(now_ms - state->motion_last_ms) < BLU2USB_MOD06_QUIET_MS)
        return;
    /* Rearm after true quiet, never just a diagonal/axis sign flicker. */
    state->gesture_latched = false;
    state->gesture_x = state->gesture_y = 0;
    state->motion_seen = false;
}

static bool has_move_assignment(const blu2usb_modifier_shortcuts_t *state)
{
    for (unsigned combo = 8u; combo <= 11u; ++combo)
        if (state->assignments[combo] != 0u) return true;
    return false;
}

static void update_tap_motion(blu2usb_modifier_shortcuts_t *state,
                              int32_t x, int32_t y)
{
    const uint32_t units = magnitude(x) + magnitude(y);
    const uint32_t total = (uint32_t)state->tap_motion + units;
    state->tap_motion = (uint8_t)(total > 8u ? 8u : total);
    if (state->tap_motion >= 8u)
        state->secondary_used = true;
}

bool blu2usb_modifier_shortcuts_process_event(
    blu2usb_modifier_shortcuts_t *state,
    const blu2usb_canonical_mouse_event_t *event,
    blu2usb_movement_settings_t settings,
    uint32_t now_ms,
    blu2usb_canonical_mouse_event_t *forwarded)
{
    if (state == NULL || event == NULL || forwarded == NULL) return false;
    *forwarded = *event; /* normal Mouse reports remain byte-preserved */
    if (event->type == BLU2USB_MOUSE_EVENT_BUTTON) {
        const int physical = physical_source(event->data.button.button);
        if (state->area_selection && state->modifier_down &&
            state->area_trigger_ready &&
            physical == (int)state->area_trigger_source) {
            const uint8_t bit = (uint8_t)(1u << (unsigned)physical);
            const bool was_pressed = (state->physical_buttons & bit) != 0u;
            if (event->data.button.pressed) state->physical_buttons |= bit;
            else state->physical_buttons &= (uint8_t)~bit;
            if (!event->data.button.pressed && state->area_drag_down) {
                state->area_drag_down = false;
                forwarded->source = area_drag_source();
                forwarded->data.button.button = BLU2USB_MOUSE_BUTTON_LEFT;
                return true;
            }
            if (event->data.button.pressed && !was_pressed &&
                !state->area_drag_down) {
                state->area_drag_down = true;
                forwarded->source = area_drag_source();
                forwarded->data.button.button = BLU2USB_MOUSE_BUTTON_LEFT;
                return true;
            }
            return false; /* duplicated physical edges are never mouse clicks */
        }
        const bool was_selection = state->area_selection;
        const bool down = state->area_drag_down;
        const bool pass = blu2usb_modifier_shortcuts_process(state, event);
        if (state->area_selection && state->modifier_down &&
            physical == (int)state->area_trigger_source &&
            !event->data.button.pressed)
            state->area_trigger_ready = true;
        if (was_selection && !state->area_selection && down) {
            /* Modifier was released during rectangle drag: no stuck LEFT. */
            state->area_drag_down = false;
            forwarded->source = area_drag_source();
            forwarded->data.button.button = BLU2USB_MOUSE_BUTTON_LEFT;
            forwarded->data.button.pressed = false;
            return true;
        }
        return pass;
    }
    if (!state->enabled || !state->modifier_down)
        return true;
    if (state->area_selection) {
        /* Spectacle is now responsible for region UI: gesture classifier
         * must NOT freeze or consume pointer movement while held. */
        if (event->type == BLU2USB_MOUSE_EVENT_MOVE) {
            if (event->data.move.dx || event->data.move.dy)
                state->secondary_used = true;
        }
        return true;
    }
    if (event->type == BLU2USB_MOUSE_EVENT_WHEEL) {
        const int32_t vertical = event->data.wheel.vertical;
        if (vertical != 0 || event->data.wheel.horizontal != 0)
            state->secondary_used = true;
        if (vertical == 0) return true; /* pure horizontal pan */
        const unsigned combo = vertical > 0 ? 6u : 7u;
        const uint8_t id = state->assignments[combo];
        if (!blu2usb_modifier_shortcuts_action_supported(id))
            return true; /* no assignment or future-only action: keep scroll */
        /* Signed canonical detents; bounded by the FIFO's remaining slots.
         * Drop excess on overflow, without leaking already captured scroll. */
        uint32_t count = magnitude(vertical);
        if (count > BLU2USB_MOD05_QUEUE_CAPACITY)
            count = BLU2USB_MOD05_QUEUE_CAPACITY;
        for (uint32_t i = 0; i < count; ++i)
            queue_action(state, id);
        forwarded->data.wheel.vertical = 0;
        return forwarded->data.wheel.horizontal != 0; /* preserve pan */
    }
    if (event->type != BLU2USB_MOUSE_EVENT_MOVE)
        return true;
    const int32_t raw_x = event->data.move.dx;
    const int32_t raw_y = event->data.move.dy;
    if (!has_move_assignment(state)) {
        update_tap_motion(state, raw_x, raw_y);
        return true; /* identical to MOD-05 when no MOVE is configured */
    }
    /* Do not write into the existing stateful Movement pipeline. It receives
     * only unconsumed events. Shared PURE rotation is used before H/V gain. */
    blu2usb_modifier_shortcuts_tick(state, now_ms);
    if (raw_x == 0 && raw_y == 0) return false;
    update_tap_motion(state, raw_x, raw_y);
    int64_t rx = 0, ry = 0;
    blu2usb_movement_rotate_delta(settings, raw_x, raw_y, &rx, &ry);
    state->motion_last_ms = now_ms;
    state->motion_seen = true;
    if (state->gesture_latched) return false; /* no auto-repeat */
    /* Significant reversal restarts unfinished gesture instead of allowing
     * two opposite swipes to cancel and fire an unintended axis later. */
    if ((state->gesture_x >= 8 && rx <= -8) ||
        (state->gesture_x <= -8 && rx >= 8))
        state->gesture_x = state->gesture_y = 0;
    if ((state->gesture_y >= 8 && ry <= -8) ||
        (state->gesture_y <= -8 && ry >= 8))
        state->gesture_x = state->gesture_y = 0;
    state->gesture_x = bounded_add(state->gesture_x, rx);
    state->gesture_y = bounded_add(state->gesture_y, ry);
    const uint32_t abs_x = magnitude(state->gesture_x);
    const uint32_t abs_y = magnitude(state->gesture_y);
    unsigned combo = BLU2USB_SHORTCUT_SLOT_COUNT;
    if (abs_x >= BLU2USB_MOD06_MOVE_THRESHOLD &&
        abs_x >= BLU2USB_MOD06_DOMINANCE_RATIO * abs_y)
        combo = state->gesture_x > 0 ? 9u : 8u; /* RIGHT / LEFT */
    else if (abs_y >= BLU2USB_MOD06_MOVE_THRESHOLD &&
             abs_y >= BLU2USB_MOD06_DOMINANCE_RATIO * abs_x)
        combo = state->gesture_y > 0 ? 11u : 10u; /* DOWN / UP */
    if (combo < BLU2USB_SHORTCUT_SLOT_COUNT) {
        state->gesture_latched = true;
        state->secondary_used = true;
        queue_action(state, state->assignments[combo]);
    }
    return false; /* freeze pointer, even before detection threshold */
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

static bool apply_modifier_mask(blu2usb_hid_aggregator_t *agg,
                                uint8_t mask)
{
    for (unsigned bit = 0u; bit < BLU2USB_MOD_COUNT; ++bit) {
        if ((mask & (uint8_t)(1u << bit)) == 0u) continue;
        blu2usb_canonical_keyboard_event_t e = {0};
        e.source = shortcut_source();
        e.type = BLU2USB_KEYBOARD_EVENT_MODIFIER;
        e.data.modifier.modifier = (blu2usb_modifier_t)bit;
        e.data.modifier.pressed = true;
        if (!blu2usb_hid_aggregator_apply_keyboard(agg, &e))
            return false;
    }
    return true;
}

/* A USB key already held by another HID source cannot create an observable
 * new key-down edge. Defer until the other source releases it, without
 * modifying or releasing any unrelated source-owned keyboard state. */
static bool keyboard_key_available(
    const blu2usb_hid_aggregator_t *agg, uint8_t key_usage)
{
    if (agg->key_refs[key_usage] != 0u) return false;
    /* At most six key usages can be represented in our 6KRO descriptor. */
    unsigned occupied = 0u;
    for (unsigned key = 1u; key < BLU2USB_HID_KEY_COUNT; ++key)
        occupied += agg->key_refs[key] != 0u;
    return occupied < 6u;
}

void blu2usb_modifier_shortcuts_prepare_usb(blu2usb_modifier_shortcuts_t *state,
    blu2usb_hid_aggregator_t *aggregator)
{
    if (state == NULL || aggregator == NULL) return;
    if (state->usb_phase == BLU2USB_MOD05_USB_NEED_UP) {
        if (state->running_consumer)
            state->consumer_usage = 0u;
        else
            (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
        state->usb_phase = BLU2USB_MOD05_USB_WAIT_UP;
        return;
    }
    if (state->usb_phase != BLU2USB_MOD05_USB_IDLE || !state->queue_size)
        return;
    const uint8_t id = state->pending[state->queue_head];
    blu2usb_keyboard_shortcut_t spec;
    uint16_t consumer = 0u;
    const bool keyboard = blu2usb_keyboard_shortcut_lookup(id, &spec);
    if (!keyboard && !blu2usb_consumer_shortcut_lookup(id, &consumer)) {
        /* Reject corrupted queue items; never emit a semantic alias. */
        state->queue_head = (uint8_t)((state->queue_head + 1u) % BLU2USB_MOD05_QUEUE_CAPACITY);
        --state->queue_size;
        return;
    }
    if (keyboard && !keyboard_key_available(aggregator, spec.key_usage))
        return; /* wait for genuine down edge and 6KRO availability */
    state->queue_head = (uint8_t)((state->queue_head + 1u) % BLU2USB_MOD05_QUEUE_CAPACITY);
    --state->queue_size;
    state->running_consumer = !keyboard;
    state->consumer_usage = consumer;
    if (keyboard && (!apply_modifier_mask(aggregator, spec.modifier_mask) ||
                     !apply_key(aggregator, spec.key_usage, true))) {
        (void)blu2usb_hid_aggregator_release_source(aggregator, shortcut_source());
        state->running_consumer = false;
        state->consumer_usage = 0u;
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
        state->running_consumer = false;
        state->consumer_usage = 0u;
    }
}

bool blu2usb_modifier_shortcuts_consumer_phase(
    const blu2usb_modifier_shortcuts_t *state)
{
    return state != NULL && state->running_consumer &&
           state->usb_phase != BLU2USB_MOD05_USB_IDLE;
}

uint16_t blu2usb_modifier_shortcuts_consumer_usage(
    const blu2usb_modifier_shortcuts_t *state)
{
    return blu2usb_modifier_shortcuts_consumer_phase(state)
        ? state->consumer_usage : 0u;
}
