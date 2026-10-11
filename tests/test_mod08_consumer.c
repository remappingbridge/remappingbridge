/* MOD-08: all eight Consumer usages + serial down/up and cancellation.
 * No claim of actual host backlight/media effects without physical testing. */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"
#include "blu2usb/modifier_shortcuts/consumer_catalog.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"
#include "blu2usb/usb_hid/usb_hid.h"

static void check_consumer(uint8_t id, uint16_t expected_usage)
{
    blu2usb_modifier_shortcuts_t state;
    blu2usb_hid_aggregator_t aggregator;
    blu2usb_modifier_shortcuts_init(&state);
    blu2usb_hid_aggregator_init(&aggregator);
    uint16_t usage=0u;
    blu2usb_keyboard_shortcut_t kb={0};
    assert(blu2usb_consumer_shortcut_lookup(id,&usage));
    assert(usage==expected_usage);
    assert(!blu2usb_keyboard_shortcut_lookup(id,&kb));
    assert(blu2usb_modifier_shortcuts_action_supported(id));
    state.pending[0]=id;state.queue_size=1u;
    blu2usb_modifier_shortcuts_prepare_usb(&state,&aggregator);
    assert(blu2usb_modifier_shortcuts_consumer_phase(&state));
    assert(blu2usb_modifier_shortcuts_consumer_usage(&state)==expected_usage);
    assert(state.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    assert(state.running_action==id && state.running_consumer);
    /* Busy USB endpoint: no premature release, modifier keys unaffected. */
    blu2usb_modifier_shortcuts_ack_usb(&state,false);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&aggregator);
    assert(state.usb_phase==BLU2USB_MOD05_USB_WAIT_DOWN);
    assert(blu2usb_modifier_shortcuts_consumer_usage(&state)==expected_usage);
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    assert(state.usb_phase==BLU2USB_MOD05_USB_NEED_UP);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&aggregator);
    assert(state.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    assert(blu2usb_modifier_shortcuts_consumer_phase(&state));
    assert(blu2usb_modifier_shortcuts_consumer_usage(&state)==0u);
    blu2usb_modifier_shortcuts_ack_usb(&state,false);
    assert(state.usb_phase==BLU2USB_MOD05_USB_WAIT_UP);
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    assert(state.usb_phase==BLU2USB_MOD05_USB_IDLE);
    assert(!state.running_consumer && !state.running_action);
    assert(!blu2usb_modifier_shortcuts_consumer_phase(&state));
    assert(state.queue_size==0u);
}
static void check_cancel_and_order(void)
{
    blu2usb_modifier_shortcuts_t state;
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_init(&state);
    blu2usb_hid_aggregator_init(&agg);
    /* Consumer + Keyboard share the FIFO; no second key until release ACK. */
    state.pending[0]=19u;state.pending[1]=1u;state.pending[2]=22u;
    state.queue_size=3u;
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    assert(state.running_consumer && state.running_action==19u);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    assert(state.running_action==19u && state.queue_size==2u);
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    assert(state.usb_phase==BLU2USB_MOD05_USB_IDLE);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    assert(!state.running_consumer && state.running_action==1u);
    blu2usb_hid_output_state_t out;
    blu2usb_hid_aggregator_snapshot(&agg,&out);
    assert(out.modifiers & 1u);
    assert(out.key_bitmap[0x06u>>3u] & (1u<<(0x06u&7u)));
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    blu2usb_modifier_shortcuts_ack_usb(&state,true);
    blu2usb_modifier_shortcuts_prepare_usb(&state,&agg);
    assert(state.running_consumer && state.running_action==22u);
    /* Disconnect/cancel releases state but transport must neutralize last
     * accepted Consumer usage (tested by source/adapter contract). */
    blu2usb_modifier_shortcuts_cancel(&state,&agg);
    assert(!state.running_consumer && state.consumer_usage==0u);
    assert(!state.queue_size && !state.running_action);
    assert(agg.modifier_refs[BLU2USB_MOD_LEFT_CTRL]==0u);
}
static void check_invalid(void)
{
    uint16_t usage=0u;
    assert(!blu2usb_consumer_shortcut_lookup(0u,&usage));
    assert(!blu2usb_consumer_shortcut_lookup(31u,&usage));
    assert(!blu2usb_consumer_shortcut_lookup(19u,NULL));
    assert(!blu2usb_modifier_shortcuts_action_supported(31u));
    assert(!blu2usb_modifier_shortcuts_action_supported(0u));
    assert(BLU2USB_USB_HID_INTERFACE_COUNT==2u);
    assert(BLU2USB_USB_REPORT_ID_KEYBOARD==1u);
    assert(BLU2USB_USB_REPORT_ID_CONSUMER==2u);
    assert(BLU2USB_USB_CONSUMER_PAYLOAD_SIZE==2u);
    assert(sizeof(blu2usb_usb_keyboard_report_t)==8u);
    assert(sizeof(blu2usb_usb_mouse_report_t)==5u);
}
int main(void)
{
    check_consumer(19u,0x00cdu);
    check_consumer(20u,0x00b5u);
    check_consumer(21u,0x00b6u);
    check_consumer(22u,0x00e9u);
    check_consumer(23u,0x00eau);
    check_consumer(24u,0x00e2u);
    check_consumer(29u,0x006fu);
    check_consumer(30u,0x0070u);
    check_cancel_and_order();
    check_invalid();
    return 0;
}
