/* MOD-10: all thirty semantic actions independently send down+up with
 * USB ACK/backpressure and no ownership left behind. Host behavioral test. */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/consumer_catalog.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/usb_hid/usb_hid.h"

static bool has_key(const blu2usb_hid_output_state_t *out, unsigned key)
{
    return (out->key_bitmap[key>>3u] & (uint8_t)(1u << (key & 7u))) != 0u;
}
static void check_action(uint8_t id)
{
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_init(&s);
    blu2usb_keyboard_shortcut_t key = {0};
    uint16_t consumer=0u;
    const bool kb=blu2usb_keyboard_shortcut_lookup(id,&key);
    const bool cc=blu2usb_consumer_shortcut_lookup(id,&consumer);
    assert(kb!=cc && (kb || cc));
    assert(blu2usb_modifier_shortcuts_action_supported(id));
    s.pending[0]=id;
    s.queue_size=1u;
    blu2usb_modifier_shortcuts_tick(&s,1000u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.running_action==id && s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    assert(s.running_consumer == cc);
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    if (cc) {
        assert(blu2usb_modifier_shortcuts_consumer_usage(&s)==consumer);
        assert(out.modifiers == 0u);
    } else {
        assert(out.modifiers == key.modifier_mask);
        assert(has_key(&out,key.key_usage));
    }
    blu2usb_modifier_shortcuts_ack_usb(&s,false); /* unavailable endpoint */
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    const uint32_t dwell=cc ? BLU2USB_MOD08_CONSUMER_HOLD_MS :
                               BLU2USB_MOD08_KEYBOARD_HOLD_MS;
    blu2usb_modifier_shortcuts_tick(&s,1000u+dwell-1u);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_tick(&s,1000u+dwell);
    blu2usb_modifier_shortcuts_prepare_usb(&s,&agg);
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    if(cc) {
        assert(blu2usb_modifier_shortcuts_consumer_usage(&s)==0u);
    } else {
        assert(!has_key(&out,key.key_usage));
        assert(out.modifiers==0u);
    }
    blu2usb_modifier_shortcuts_ack_usb(&s,false); /* busy on key-up */
    assert(s.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&s,true);
    assert(s.usb_phase==BLU2USB_MOD05_USB_IDLE);
    assert(!s.running_action && !s.queue_size);
    assert(agg.modifier_refs[BLU2USB_MOD_LEFT_CTRL]==0u);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
}
int main(void)
{
    assert(BLU2USB_SHORTCUT_COUNT==30u);
    assert(BLU2USB_SHORTCUT_SLOT_COUNT==12u);
    assert(BLU2USB_USB_HID_INTERFACE_COUNT==2u);
    assert(sizeof(blu2usb_usb_keyboard_report_t)==8u);
    assert(sizeof(blu2usb_usb_mouse_report_t)==5u);
    for (unsigned id=1;id<=30;id++) check_action((uint8_t)id);
    for (unsigned id=31;id<256;id++)
        assert(!blu2usb_modifier_shortcuts_action_supported((uint8_t)id));
    return 0;
}
