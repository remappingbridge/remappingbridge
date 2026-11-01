#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "contacts.h"
#include <stdio.h>
#include <string.h>

extern uint8_t tp_mode, tp_switches;
static uint8_t descriptor_storage[4096];
static btstack_packet_callback_registration_t registration;
static bd_addr_t target;
static bool target_known, scanning, connecting, working, dirty, streaming;
static uint16_t cid;
static uint32_t next_attempt, enable_deadline;
static tp_frame latest;
static tp_report delivered;

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
    connecting = streaming = false;
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
        case HID_SUBEVENT_INCOMING_CONNECTION:
            hid_host_decline_connection(hid_subevent_incoming_connection_get_hid_cid(p));
            break;
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
        case HID_SUBEVENT_DESCRIPTOR_AVAILABLE: {
            static const uint8_t enable[] = {2,1};
            if (cid && (hid_subevent_descriptor_available_get_status(p) ||
                hid_host_send_set_report(cid,HID_REPORT_TYPE_FEATURE,0xf1,enable,sizeof(enable))))
                hid_host_disconnect(cid);
            break;
        }
        case HID_SUBEVENT_REPORT: {
            const uint8_t *r = hid_subevent_report_get_report(p);
            unsigned n = hid_subevent_report_get_report_len(p);
            if (cid && n && r[0] == 0xa1 && tp_decode(r+1,n-1,&latest)) {
                if (!streaming) printf("Apple multitouch active\n");
                streaming = dirty = true;
            }
            break;
        }
        case HID_SUBEVENT_SET_REPORT_RESPONSE:
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
    if (cid && !streaming && (int32_t)(now-enable_deadline) >= 0) {
        printf("Waiting for first touch report\n");
        enable_deadline = now + 10000;
    }
    if (!working || scanning || connecting || cid || (int32_t)(now-next_attempt)<0) return;
    next_attempt = now + 2500;
    if (target_known) {
        connecting = hid_host_connect(target,HID_PROTOCOL_MODE_REPORT,&cid) == ERROR_CODE_SUCCESS;
        if (!connecting) reset_session();
    } else scanning = gap_inquiry_start(4) == ERROR_CODE_SUCCESS;
}

static void usb_task(void) {
    static uint8_t last_mode = 255, last_switches;
    if (!tud_mounted()) {
        delivered = (tp_report){0};
        dirty = true;
        last_mode = 255;
        return;
    }
    if (tp_mode != last_mode || tp_switches != last_switches) dirty = true;
    if (!dirty || !tud_hid_ready()) return;
    tp_frame f = latest;
    if (!(tp_switches & 1)) memset(f.finger,0,sizeof(f.finger));
    if (!(tp_switches & 2)) f.click = false;
    if (tp_mode == 0) {
        /* Relative pointer compatibility when the host selects mouse mode. */
        static int prior_id = -1;
        static tp_contact prior;
        if (last_mode != 0) prior_id = -1;
        int id = 0;
        while (id < 16 && !f.finger[id].down) id++;
        int dx=0, dy=0;
        if (id < 16 && id == prior_id) {
            dx=((int)f.finger[id].x-prior.x)/8;
            dy=((int)f.finger[id].y-prior.y)/8;
        }
        if (dx > 127) dx=127;
        if (dx < -127) dx=-127;
        if (dy > 127) dy=127;
        if (dy < -127) dy=-127;
        if (!tud_hid_mouse_report(6,f.click,dx,dy,0,0)) return;
        prior_id=id;
        if (id < 16) prior=f.finger[id];
        delivered=(tp_report){0};
    } else {
        tp_report r;
        tp_encode(&f,&delivered,(uint16_t)(time_us_64()/100),&r);
        if (!tud_hid_report(1,&r,sizeof(r))) return;
        delivered=r;
    }
    last_switches=tp_switches;
    last_mode=tp_mode;
    dirty=false;
}

int main(void) {
    stdio_init_all();
    if (cyw43_arch_init()) return 1;
    tusb_init();
    if (TRACKPAD_ADDRESS[0]) target_known = sscanf_bd_addr(TRACKPAD_ADDRESS,target);
    l2cap_init();
    hid_host_init(descriptor_storage,sizeof(descriptor_storage));
    hid_host_register_packet_handler(packet);
    registration.callback=packet;
    hci_add_event_handler(&registration);
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    hci_power_control(HCI_POWER_ON);
    while (true) {
        cyw43_arch_poll();
        bluetooth_task();
        tud_task();
        usb_task();
    }
}
