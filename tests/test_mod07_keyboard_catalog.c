/* MOD-07 Linux-first 22-action keyboard mapping; Release asserts enabled. */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"

typedef struct { uint8_t id, usage, mods; } expected_t;
static const expected_t k_expected[] = {
    {1, 0x06, 0x01}, {2, 0x1b, 0x01}, {3, 0x04, 0x01},
    {4, 0x19, 0x01}, {5, 0x1d, 0x01}, {6, 0x1d, 0x03},
    {7, 0x29, 0x00}, {8, 0x2c, 0x00}, {9, 0x28, 0x00},
    {10, 0x2a, 0x00}, {11, 0x4c, 0x00}, {12, 0x3b, 0x00},
    {13, 0x44, 0x00}, {14, 0x46, 0x00}, {15, 0x46, 0x02},
    {16, 0x15, 0x01}, {17, 0x09, 0x01}, {18, 0x16, 0x01},
    {25, 0x4a, 0x00}, {26, 0x57, 0x01},
    {27, 0x56, 0x01}, {28, 0x27, 0x01}
};
static bool key_down(const blu2usb_hid_output_state_t *out,unsigned key) {
    return !!(out->key_bitmap[key>>3u]&(1u<<(key&7u)));
}
static void run_action(uint8_t id,uint8_t usage,uint8_t modifiers)
{
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_init(&s);
    s.pending[0] = id; s.queue_size=1;
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.running_action==id && s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(key_down(&out,usage) && out.modifiers==modifiers);
    /* Stalled USB endpoint: press remains stable, no premature release. */
    blu2usb_modifier_shortcuts_ack_usb(&s,false);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(key_down(&out,usage) && out.modifiers==modifiers);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(!key_down(&out,usage) && out.modifiers==0);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&s,false);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_IDLE);
    assert(s.queue_size==0 && s.running_action==0);
}
static void keyboard_ownership_and_kro(void)
{
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_hid_source_t external=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_KEYBOARD,7u);
    blu2usb_canonical_keyboard_event_t e={0};
    e.source=external;e.type=BLU2USB_KEYBOARD_EVENT_KEY;e.data.key.pressed=true;
    e.data.key.key=0x06u; /* C held by independent keyboard source */
    assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&e));
    s.pending[0]=1u;s.queue_size=1u;
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.queue_size==1 && s.usb_phase==BLU2USB_MOD05_USB_IDLE);
    e.data.key.pressed=false;
    assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&e));
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(agg.key_refs[0x06]==0u && agg.modifier_refs[0]==0u);

    /* All six report positions occupied: delay another shortcut rather
     * than pretending it was sent in a 6KRO report. */
    for(unsigned u=0x20;u<0x26;u++) {
        e.data.key.key=(uint8_t)u;e.data.key.pressed=true;
        assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&e));
    }
    s.pending[0]=7u;s.queue_size=1u;
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.queue_size==1 && s.usb_phase==BLU2USB_MOD05_USB_IDLE);
    e.data.key.key=0x20u;e.data.key.pressed=false;
    assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&e));
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    for(unsigned u=0x21;u<0x26;u++)
        assert(key_down(&out,u));
    assert(!key_down(&out,0x29u));
    for(unsigned u=0x21;u<0x26;u++) {
        e.data.key.key=(uint8_t)u;e.data.key.pressed=false;
        assert(blu2usb_hid_aggregator_apply_keyboard(&agg,&e));
    }
}
static void FIFO_and_cancel(void)
{
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_init(&s);
    s.pending[0]=2u;s.pending[1]=4u;s.pending[2]=6u;s.queue_size=3;
    const uint8_t ids[3]={2u,4u,6u};
    for(unsigned i=0;i<3;i++) {
        blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
        assert(s.running_action==ids[i] && s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
        blu2usb_modifier_shortcuts_ack_usb(&s,true);
        blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
        assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
        blu2usb_modifier_shortcuts_ack_usb(&s,true);
        assert(s.usb_phase==BLU2USB_MOD05_USB_IDLE);
    }
    assert(s.queue_size==0);
    s.pending[0]=15u;s.queue_head=0;s.queue_size=1;
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(agg.modifier_refs[BLU2USB_MOD_LEFT_SHIFT]!=0u);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(agg.modifier_refs[BLU2USB_MOD_LEFT_SHIFT]==0u);
    assert(agg.key_refs[0x46u]==0u);
}
int main(void)
{
    unsigned count=0;
    for(unsigned id=0;id<=255;++id) {
        blu2usb_keyboard_shortcut_t chord={0};
        bool found=blu2usb_keyboard_shortcut_lookup((uint8_t)id,&chord);
        bool supported=blu2usb_modifier_shortcuts_action_supported((uint8_t)id);
        assert(found==supported);
        if(!found) continue;
        assert(chord.key_usage!=0 && (chord.modifier_mask&0xfcu)==0);
        bool matched=false;
        for(unsigned i=0;i<sizeof(k_expected)/sizeof(k_expected[0]);i++) {
            const expected_t *e=&k_expected[i];
            if(e->id!=id)continue;
            assert(chord.key_usage==e->usage && chord.modifier_mask==e->mods);
            matched=true;
        }
        assert(matched);
        run_action((uint8_t)id,chord.key_usage,chord.modifier_mask);
        count++;
    }
    assert(count==22u);
    assert(!blu2usb_keyboard_shortcut_lookup(0,NULL));
    for(unsigned id=19;id<=24;id++)
        assert(!blu2usb_modifier_shortcuts_action_supported((uint8_t)id));
    assert(!blu2usb_modifier_shortcuts_action_supported(29u));
    assert(!blu2usb_modifier_shortcuts_action_supported(30u));
    keyboard_ownership_and_kro();
    FIFO_and_cancel();
    return 0;
}
