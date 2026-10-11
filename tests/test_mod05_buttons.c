/* MOD-05 physical-button and ACK-gated HID regression (Release assertions ON). */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/usb_hid/usb_hid.h"

static blu2usb_canonical_mouse_event_t button(unsigned b, bool pressed)
{
    blu2usb_canonical_mouse_event_t e={0};
    e.source=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE,1u);
    e.type=BLU2USB_MOUSE_EVENT_BUTTON;
    e.data.button.button=(blu2usb_mouse_button_t)b;
    e.data.button.pressed=pressed;
    return e;
}
static bool send_button(blu2usb_modifier_shortcuts_t *state,unsigned b,bool pressed)
{
    blu2usb_canonical_mouse_event_t e=button(b,pressed);
    return blu2usb_modifier_shortcuts_process(state,&e);
}
static bool key(const blu2usb_hid_output_state_t *out,unsigned usage)
{
    return (out->key_bitmap[usage>>3u] & (1u<<(usage&7u)))!=0;
}
static void report(blu2usb_hid_aggregator_t *agg,
                   blu2usb_usb_keyboard_report_t *usb)
{
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(agg,&out);
    uint8_t keys[6]={0};
    unsigned cursor=0;
    for(unsigned k=0;k<256 && cursor<6;++k)
        if(key(&out,k))keys[cursor++]=(uint8_t)k;
    blu2usb_usb_hid_build_keyboard_report(usb,out.modifiers,keys);
}
static void idle(const blu2usb_modifier_shortcuts_t *s)
{
    assert(s->usb_phase==BLU2USB_MOD05_USB_IDLE);
    assert(s->running_action==0);
}
static void acknowledge_action(blu2usb_modifier_shortcuts_t *s,
                               blu2usb_hid_aggregator_t *agg,
                               uint8_t id)
{
    blu2usb_usb_keyboard_report_t usb;
    blu2usb_modifier_shortcuts_prepare_usb(s,agg);
    assert(s->usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    assert(s->running_action==id);
    report(agg,&usb);
    if(id==7u) {
        assert(usb.keycodes[0]==0x29u);
        assert(usb.modifiers==0);
    } else {
        assert(id==1u);
        assert(usb.keycodes[0]==0x06u);
        assert(usb.modifiers==1u);
    }
    /* A busy USB endpoint can NOT advance phases nor release the key. */
    blu2usb_modifier_shortcuts_ack_usb(s,false);
    blu2usb_modifier_shortcuts_prepare_usb(s,agg);
    assert(s->usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    report(agg,&usb);
    assert(usb.keycodes[0]==(id==7u?0x29u:0x06u));

    blu2usb_modifier_shortcuts_ack_usb(s,true);
    assert(s->usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    /* The release report must occur on a separate USB service tick. */
    blu2usb_modifier_shortcuts_prepare_usb(s,agg);
    assert(s->usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    report(agg,&usb);
    assert(usb.modifiers==0 && usb.keycodes[0]==0);

    blu2usb_modifier_shortcuts_ack_usb(s,false);
    assert(s->usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(s,true);
    idle(s);
}
static void configure(blu2usb_modifier_shortcuts_t *s,
                      blu2usb_hid_aggregator_t *agg,
                      unsigned modifier, const uint8_t assigned[12])
{
    blu2usb_mouse_profile_config_t profile={0};
    profile.kind=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    const blu2usb_mouse_target_t normal[5]={
        BLU2USB_MOUSE_TARGET_LEFT,BLU2USB_MOUSE_TARGET_RIGHT,
        BLU2USB_MOUSE_TARGET_MIDDLE,BLU2USB_MOUSE_TARGET_FORWARD,
        BLU2USB_MOUSE_TARGET_BACKWARD
    };
    memcpy(profile.targets,normal,sizeof normal);
    if(modifier<BLU2USB_MOUSE_SOURCE_COUNT)
        profile.targets[modifier]=BLU2USB_MOUSE_TARGET_MODIFIER;
    blu2usb_modifier_shortcuts_configure(s,&profile,assigned,agg);
}
static void test_solo_release_escape_and_hold(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7;
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(s.enabled);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    /* A second down while held is NOT an auto-repeat */
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(!s.queue_size);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(s.queue_size==1u);
    acknowledge_action(&s,&agg,7u);
    assert(s.queue_size==0u);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(!s.queue_size);
}
static void test_copy_consumes_down_up_without_tap(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7;
    assigned[3]=1; /* physical RIGHT */
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true)); /* repeated down */
    assert(s.queue_size==1u);
    /* Releasing Modifier before RIGHT must not leak a late right-button up. */
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,false));
    assert(s.queue_size==1u); /* precisely COPY, never extra ESCAPE */
    acknowledge_action(&s,&agg,1u);
}
static void test_unmapped_secondary_passes_and_suppresses_tap(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7;
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,false));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(s.queue_size==0u);
    /* When Modifier is not held a normal drag can still be delivered. */
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,false));
    /* Future-supported labels may be saved, but must not misfire as another key. */
    assigned[3]=19; /* Consumer PLAY PAUSE: MOD-08 */
    blu2usb_modifier_shortcuts_assignments(&s,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,false));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(s.queue_size==0u);
}
static void test_passthrough_disabled_and_masks(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7; assigned[2]=1; assigned[3]=1;
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(s.queue_size==0u);
    blu2usb_mouse_profile_config_t off={0};
    off.kind=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    blu2usb_modifier_shortcuts_configure(&s,&off,assigned,&agg);
    assert(!s.enabled);
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    assert(!s.queue_size);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(s.queue_size==1u); /* only RELEASED; its same-source LEFT combo masked */
    acknowledge_action(&s,&agg,7u);
}
static void test_usb_queue_and_cancel_no_stuck_keys(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7;assigned[3]=1;
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    for(unsigned i=0;i<3;++i) {
        assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
        assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
        assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,false));
        assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    }
    assert(s.queue_size==3u);
    acknowledge_action(&s,&agg,1u);
    acknowledge_action(&s,&agg,1u);
    acknowledge_action(&s,&agg,1u);
    assert(s.queue_size==0u);

    /* Source-scoped ownership: cancel must not release unrelated keyboard key. */
    blu2usb_canonical_keyboard_event_t other={0};
    other.source=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_KEYBOARD,2u);
    other.type=BLU2USB_KEYBOARD_EVENT_KEY;
    other.data.key.key=BLU2USB_KEY_A;
    other.data.key.pressed=true;
    assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&other));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_RIGHT,true));
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(key(&out,BLU2USB_KEY_A) && key(&out,0x06u));
    blu2usb_modifier_shortcuts_cancel(&s,&agg); /* disconnect/lock/profile */
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(key(&out,BLU2USB_KEY_A));
    assert(!key(&out,0x06u));
    assert(out.modifiers==0u);
    assert(!s.queue_size && !s.modifier_down && !s.consumed_buttons);
    idle(&s);
    other.data.key.pressed=false;
    assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&other));
}
static void test_wheel_and_motion_forward_until_mod06(void)
{
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    uint8_t assigned[12]={0};
    assigned[0]=7;assigned[6]=1;assigned[8]=7;
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_aggregator_init(&agg);
    configure(&s,&agg,BLU2USB_MOUSE_SOURCE_LEFT,assigned);
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,true));
    blu2usb_canonical_mouse_event_t e=button(0,false);
    e.type=BLU2USB_MOUSE_EVENT_WHEEL;
    e.data.wheel.vertical=1;
    assert(blu2usb_modifier_shortcuts_process(&s,&e));
    e.type=BLU2USB_MOUSE_EVENT_MOVE;
    e.data.move.dx=5;e.data.move.dy=-3;
    assert(blu2usb_modifier_shortcuts_process(&s,&e));
    assert(!send_button(&s,BLU2USB_MOUSE_BUTTON_LEFT,false));
    assert(!s.queue_size); /* second gestures suppress release, but do not trigger */
}
int main(void)
{
    assert(blu2usb_modifier_shortcuts_action_supported(1u));
    assert(blu2usb_modifier_shortcuts_action_supported(7u));
    assert(!blu2usb_modifier_shortcuts_action_supported(19u));
    test_solo_release_escape_and_hold();
    test_copy_consumes_down_up_without_tap();
    test_unmapped_secondary_passes_and_suppresses_tap();
    test_passthrough_disabled_and_masks();
    test_usb_queue_and_cancel_no_stuck_keys();
    test_wheel_and_motion_forward_until_mod06();
    return 0;
}
