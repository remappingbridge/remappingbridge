#include "btstack.h"
#include "btstack_tlv.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "usb.h"
#include "contacts.h"
#include <stdio.h>
#include <string.h>


static uint8_t descriptor_storage[4096];
static btstack_packet_callback_registration_t registration;
/* Restore only devices that have actually produced a valid Apple frame. */
#define TRACKPAD_TLV_TAG 0x54504144u
static bd_addr_t target, saved;
static const btstack_tlv_t *tlv;
static void *tlv_context;
static bool descriptor_ready, battery_probe_pending;
static const uint8_t enable_multitouch[] = {2, 1};
static bool target_known, scanning, connecting, working, dirty, streaming;
static uint16_t cid;
static uint32_t next_attempt, enable_deadline;
static tp_frame latest;


static bool matches_trackpad(const uint8_t *event) {
    if (gap_event_inquiry_result_get_device_id_available(event)) {
        unsigned vendor = gap_event_inquiry_result_get_device_id_vendor_id(event);
        unsigned pid = gap_event_inquiry_result_get_device_id_product_id(event);
        if ((vendor == 0x004c || vendor == 0x05ac) && (pid == 0x0265 || pid == 0x0324))
            return true;
    }
    if (!gap_event_inquiry_result_get_name_available(event)) return false;
    const uint8_t *name = gap_event_inquiry_result_get_name(event);
    unsigned len = gap_event_inquiry_result_get_name_len(event);
    const char label[] = "Magic Trackpad";
    for (unsigned i = 0; i + sizeof(label)-1 <= len; i++)
        if (!memcmp(name+i,label,sizeof(label)-1)) return true;
    return false;
}

static void reset_session(void) {
    cid = 0;
    connecting = streaming = descriptor_ready = battery_probe_pending = false;
#ifdef TRACKPAD_MACOS
    trackpad_usb_reset_delivery();
#endif
    latest = (tp_frame){0};
    dirty = true;
    next_attempt = btstack_run_loop_get_time_ms() + 2500;
}

static void packet(uint8_t type, uint16_t channel, uint8_t *p, uint16_t size) {
    (void)channel; (void)size;
    if (type != HCI_EVENT_PACKET) return;
    switch (hci_event_packet_get_type(p)) {
    case BTSTACK_EVENT_STATE:
        working = btstack_event_state_get_state(p) == HCI_STATE_WORKING;
        break;
    case GAP_EVENT_INQUIRY_RESULT:
        if (scanning && !target_known && matches_trackpad(p)) {
            gap_event_inquiry_result_get_bd_addr(p,target);
            target_known = true;
            printf("Found trackpad %s\n",bd_addr_to_str(target));
            /* Let inquiry finish before opening SDP / HID channels. */
        }
        break;
    case GAP_EVENT_INQUIRY_COMPLETE:
        scanning = false;
        break;
    case HCI_EVENT_USER_CONFIRMATION_REQUEST: {
        bd_addr_t peer;
        hci_event_user_confirmation_request_get_bd_addr(p,peer);
        if (target_known && !memcmp(peer,target,sizeof(peer))) gap_ssp_confirmation_response(peer);
        break;
    }
    case HCI_EVENT_HID_META:
        switch (hci_event_hid_meta_get_subevent_code(p)) {
        case HID_SUBEVENT_INCOMING_CONNECTION: {
            bd_addr_t peer;
            hid_subevent_incoming_connection_get_address(p,peer);
            uint16_t incoming = hid_subevent_incoming_connection_get_hid_cid(p);
            if (!cid && !connecting && target_known && !memcmp(peer,target,sizeof(peer))) {
                connecting = hid_host_accept_connection(incoming,HID_PROTOCOL_MODE_REPORT) == ERROR_CODE_SUCCESS;
            } else hid_host_decline_connection(incoming);
            break;
        }
        case HID_SUBEVENT_CONNECTION_OPENED:
            connecting = false;
            if (hid_subevent_connection_opened_get_status(p)) {
                printf("HID open failed: %u\n",hid_subevent_connection_opened_get_status(p));
                reset_session();
                break;
            }
            cid = hid_subevent_connection_opened_get_hid_cid(p);
            enable_deadline = btstack_run_loop_get_time_ms() + 5000;
            printf("Classic HID connected\n");
            break;
        case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
            descriptor_ready = hid_subevent_descriptor_available_get_status(p) == 0;
            if (!descriptor_ready && cid) hid_host_disconnect(cid);
            enable_deadline = btstack_run_loop_get_time_ms();
            break;
        case HID_SUBEVENT_REPORT: {
            const uint8_t *r = hid_subevent_report_get_report(p);
            unsigned n = hid_subevent_report_get_report_len(p);
            if (cid && n && r[0] == 0xa1 && tp_decode(r+1,n-1,&latest)) {
                if (!streaming) {
                    printf("Apple multitouch active\n");
                    if (tlv && memcmp(saved,target,sizeof(saved)) &&
                        !tlv->store_tag(tlv_context,TRACKPAD_TLV_TAG,target,sizeof(target)))
                        memcpy(saved,target,sizeof(saved));
                }
                streaming = dirty = true;
            }
            break;
        }
        case HID_SUBEVENT_SET_REPORT_RESPONSE:
            if (cid && !hid_subevent_set_report_response_get_handshake_status(p))
                battery_probe_pending=true;
            if (hid_subevent_set_report_response_get_handshake_status(p))
                printf("Multitouch feature handshake: %u\n",hid_subevent_set_report_response_get_handshake_status(p));
            break;
        case HID_SUBEVENT_CONNECTION_CLOSED: reset_session(); break;
        }
        break;
    }
}

static void bluetooth_task(void) {
    uint32_t now = btstack_run_loop_get_time_ms();
    /* BTstack restores the established state after the SET_REPORT callback. */
    if (cid && battery_probe_pending &&
        !hid_host_send_get_report(cid,HID_REPORT_TYPE_INPUT,0x90)) battery_probe_pending=false;
    if (cid && descriptor_ready && !streaming && (int32_t)(now-enable_deadline) >= 0) {
        uint8_t status = hid_host_send_set_report(cid,HID_REPORT_TYPE_FEATURE,0xf1,
                                                   enable_multitouch,sizeof(enable_multitouch));
        printf("Enable multitouch: %u (touch the surface)\n",status);
        enable_deadline = now + 10000;
    }
    if (!working || scanning || connecting || cid || (int32_t)(now-next_attempt)<0) return;
    next_attempt = now + 2500;
    if (target_known) {
        connecting = hid_host_connect(target,HID_PROTOCOL_MODE_REPORT,&cid) == ERROR_CODE_SUCCESS;
        if (!connecting) reset_session();
    } else scanning = gap_inquiry_start(4) == ERROR_CODE_SUCCESS;
}

int main(void) {
    stdio_init_all();
    if (cyw43_arch_init()) return 1;
    trackpad_usb_init();
    btstack_tlv_get_instance(&tlv,&tlv_context);
    if (tlv && tlv->get_tag(tlv_context,TRACKPAD_TLV_TAG,saved,sizeof(saved)) == sizeof(saved)) {
        memcpy(target,saved,sizeof(target));
        target_known = true;
    }
    if (TRACKPAD_ADDRESS[0]) target_known = sscanf_bd_addr(TRACKPAD_ADDRESS,target);
    l2cap_init();
    hid_host_init(descriptor_storage,sizeof(descriptor_storage));
    hid_host_register_packet_handler(packet);
    registration.callback=packet;
    hci_add_event_handler(&registration);
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);
    gap_connectable_control(1);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    hci_power_control(HCI_POWER_ON);
    bool led = false;
    while (true) {
        cyw43_arch_poll();
        bluetooth_task();
        bool light = streaming || (cid && ((btstack_run_loop_get_time_ms() / 350) & 1));
#ifdef TRACKPAD_MACOS
        if (streaming && !trackpad_usb_delivered())
            light = (btstack_run_loop_get_time_ms() / 100) & 1;
#endif
        if (light != led) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,light);
            led = light;
        }
        trackpad_usb_task(&latest,&dirty);
    }
}
