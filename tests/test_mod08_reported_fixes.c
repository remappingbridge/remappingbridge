/* Operator-reported KDE/Android correction gate: triggers, synthetic pointer drag
 * and bounded non-blocking Consumer key-down dwell. Host asserts enabled. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/consumer_catalog.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/movement/movement.h"

static blu2usb_canonical_mouse_event_t button(blu2usb_mouse_button_t b,bool down)
{
    blu2usb_canonical_mouse_event_t e={0};
    e.source=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE,1);
    e.type=BLU2USB_MOUSE_EVENT_BUTTON;
    e.data.button.button=b;e.data.button.pressed=down;
    return e;
}
static blu2usb_canonical_mouse_event_t move(int16_t x,int16_t y)
{
    blu2usb_canonical_mouse_event_t e=button(BLU2USB_MOUSE_BUTTON_LEFT,false);
    e.type=BLU2USB_MOUSE_EVENT_MOVE;e.data.move.dx=x;e.data.move.dy=y;
    return e;
}
static bool flow(blu2usb_modifier_shortcuts_t *s,blu2usb_canonical_mouse_event_t e,
                 blu2usb_canonical_mouse_event_t *out)
{
    return blu2usb_modifier_shortcuts_process_event(s,&e,
        blu2usb_movement_defaults(),100,out);
}
static void start(blu2usb_modifier_shortcuts_t *s,blu2usb_hid_aggregator_t *agg)
{
    blu2usb_mouse_profile_config_t p={0};
    p.kind=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    p.targets[BLU2USB_MOUSE_SOURCE_LEFT]=BLU2USB_MOUSE_TARGET_MODIFIER;
    uint8_t ids[BLU2USB_SHORTCUT_SLOT_COUNT]={0};
    ids[0]=7u;ids[3]=15u;ids[8]=1u; /* MOVE intentionally configured */
    blu2usb_modifier_shortcuts_init(s);
    blu2usb_hid_aggregator_init(agg);
    blu2usb_modifier_shortcuts_configure(s,&p,ids,agg);
    assert(s->enabled);
}
static void check_catalog(void)
{
    blu2usb_keyboard_shortcut_t k;
    uint16_t consumer=0;
    assert(blu2usb_keyboard_shortcut_lookup(15,&k));
    assert(k.key_usage==0x46u && k.modifier_mask==0x0au); /* Meta Shift Print */
    assert(blu2usb_keyboard_shortcut_lookup(26,&k));
    assert(k.key_usage==0x2eu && k.modifier_mask==0x03u); /* Ctrl Shift '=' */
    assert(blu2usb_keyboard_shortcut_lookup(27,&k));
    assert(k.key_usage==0x2du && k.modifier_mask==0x01u); /* Ctrl '-' */
    assert(blu2usb_keyboard_shortcut_lookup(28,&k));
    assert(k.key_usage==0x27u && k.modifier_mask==0x01u);
    assert(!blu2usb_keyboard_shortcut_lookup(25,&k));
    assert(blu2usb_consumer_shortcut_lookup(25,&consumer));
    assert(consumer==0x0223u); /* Linux KEY_HOMEPAGE -> Android KEYCODE_HOME */
    for(unsigned id=19;id<=24;id++)
        assert(blu2usb_modifier_shortcuts_action_supported((uint8_t)id));
    assert(blu2usb_modifier_shortcuts_action_supported(29));
    assert(blu2usb_modifier_shortcuts_action_supported(30));
}
static void check_selection_drag(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    start(&s,&agg);
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_LEFT,true),&out));
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(s.queue_size==1 && s.pending[s.queue_head]==15);
    assert(s.area_selection && !s.area_trigger_ready);
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,false),&out));
    assert(s.area_trigger_ready);
    /* MOVE assignment is active but Spectacle must still receive motion. */
    assert(flow(&s,move(22,-16),&out));
    assert(out.data.move.dx==22 && out.data.move.dy==-16);
    assert(!s.gesture_latched);
    /* The SAME secondary button re-pressed becomes left-drag while MOD held. */
    assert(flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(out.source.kind==BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT);
    assert(out.source.instance==2);
    assert(out.data.button.button==BLU2USB_MOUSE_BUTTON_LEFT);
    assert(out.data.button.pressed);
    assert(blu2usb_hid_aggregator_apply_mouse(&agg,&out));
    assert(agg.mouse_button_refs[BLU2USB_MOUSE_BUTTON_LEFT]==1u);
    /* HID repeated press is not a second synthetic click. */
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(flow(&s,move(35,40),&out)); /* dragging motion still reaches pointer */
    assert(flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,false),&out));
    assert(!out.data.button.pressed);
    assert(out.data.button.button==BLU2USB_MOUSE_BUTTON_LEFT);
    assert(blu2usb_hid_aggregator_apply_mouse(&agg,&out));
    assert(agg.mouse_button_refs[BLU2USB_MOUSE_BUTTON_LEFT]==0u);
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_LEFT,false),&out));
    assert(!s.area_selection && !s.modifier_down);
    assert(s.queue_size==1u); /* no trailing Mod Release Escape */

    /* Even if modifier is released DURING drag, release synthetic LEFT. */
    start(&s,&agg);
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_LEFT,true),&out));
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,false),&out));
    assert(flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(blu2usb_hid_aggregator_apply_mouse(&agg,&out));
    assert(flow(&s,button(BLU2USB_MOUSE_BUTTON_LEFT,false),&out));
    assert(out.source.instance==2 && !out.data.button.pressed);
    assert(blu2usb_hid_aggregator_apply_mouse(&agg,&out));
    assert(agg.mouse_button_refs[BLU2USB_MOUSE_BUTTON_LEFT]==0u);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(agg.mouse_button_refs[BLU2USB_MOUSE_BUTTON_LEFT]==0u);

    /* Cancel on disconnect releases the drag without waiting for BLE up. */
    start(&s,&agg);
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_LEFT,true),&out));
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(!flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,false),&out));
    assert(flow(&s,button(BLU2USB_MOUSE_BUTTON_RIGHT,true),&out));
    assert(blu2usb_hid_aggregator_apply_mouse(&agg,&out));
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(agg.mouse_button_refs[BLU2USB_MOUSE_BUTTON_LEFT]==0u);
}
static void check_dwell(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_init(&s);
    s.pending[0]=22u;s.queue_size=1u;
    blu2usb_modifier_shortcuts_tick(&s,100u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.running_consumer && s.consumer_usage==0x00e9u);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_tick(&s,159u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_tick(&s,160u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    assert(s.consumer_usage==0u);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_IDLE);

    blu2usb_modifier_shortcuts_init(&s);
    s.pending[0]=15u;s.queue_size=1u;
    blu2usb_modifier_shortcuts_tick(&s,100u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    blu2usb_modifier_shortcuts_tick(&s,124u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_tick(&s,125u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_IDLE);
}
int main(void)
{
    check_catalog();
    check_selection_drag();
    check_dwell();
    return 0;
}
