#include "blu2usb/ble_hogp/ble_hogp.h"

#include <stdatomic.h>
#include <string.h>
#include "btstack.h"
#include "ble/le_device_db.h"

#define BLE_HOGP_DESCRIPTOR_STORAGE_SIZE 4096u
#define BLE_HOGP_REJECTED_DEVICE_CAPACITY 4u
#define BLE_APPEARANCE_HID_GENERIC 960u
#define BLE_APPEARANCE_HID_MOUSE 962u
#define BLE_APPEARANCE_HID_LAST 1023u
#define BLE_HOGP_VENDOR_SERVICE_MS 20u
#define BLE_HOGP_BONDED_RECONNECT_TIMEOUT_MS 8000u
#define BLE_HOGP_PAIR_NEW_TIMEOUT_MS 15000u

_Static_assert(sizeof(blu2usb_canonical_mouse_event_t) <= BLU2USB_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE,
               "canonical mouse event must fit runtime message");

typedef enum {
    BLE_HOGP_STATE_WAITING_FOR_STACK = 0,
    BLE_HOGP_STATE_IDLE,
    BLE_HOGP_STATE_SCANNING,
    BLE_HOGP_STATE_CONNECTING,
    BLE_HOGP_STATE_SECURING,
    BLE_HOGP_STATE_CONNECTING_HIDS,
    BLE_HOGP_STATE_READY,
    BLE_HOGP_STATE_DISCONNECTING,
} ble_hogp_state_t;

typedef enum {
    BLE_PAIR_NEW_IDLE = 0,
    BLE_PAIR_NEW_SCANNING,
    BLE_PAIR_NEW_CONNECTING,
    BLE_PAIR_NEW_SECURING,
    BLE_PAIR_NEW_CONNECTING_HIDS,
    BLE_PAIR_NEW_READY,
    BLE_PAIR_NEW_DISCONNECTING,
} ble_pair_new_state_t;

typedef struct {
    bool used;
    bd_addr_type_t address_type;
    bd_addr_t address;
} ble_hogp_rejected_device_t;

static ble_hogp_state_t g_state;
static bd_addr_t g_remote_address;
static bd_addr_type_t g_remote_address_type;
static hci_con_handle_t g_connection_handle = HCI_CON_HANDLE_INVALID;
static uint16_t g_hids_cid;
static uint8_t g_descriptor_storage[BLE_HOGP_DESCRIPTOR_STORAGE_SIZE];
static blu2usb_ble_hogp_parser_t g_parser;
static ble_hogp_rejected_device_t g_rejected_devices[BLE_HOGP_REJECTED_DEVICE_CAPACITY];
static size_t g_rejected_next;
static btstack_packet_callback_registration_t g_hci_registration;
static btstack_packet_callback_registration_t g_sm_registration;
static btstack_timer_source_t g_vendor_timer;
static btstack_timer_source_t g_reconnect_timer;
static bool g_reconnect_timer_active;
static bool g_reconnect_cancel_pending;
static bool g_reconnect_after_disconnect;
static bool g_idle_after_disconnect;
static bool g_saved_search_active;
static atomic_bool g_saved_search_request = ATOMIC_VAR_INIT(false);
static atomic_bool g_saved_search_cancel = ATOMIC_VAR_INIT(false);

static ble_pair_new_state_t g_pair_new_state;
static bool g_pair_new_active;
static bool g_pair_new_cancel_pending;
static bool g_pair_new_resume_after_disconnect;
static bool g_pair_new_handoff_pending;
static int g_pair_new_bond_count_before;
static bd_addr_t g_pair_new_address;
static bd_addr_type_t g_pair_new_address_type;
static hci_con_handle_t g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
static uint16_t g_pair_new_hids_cid;
static blu2usb_ble_hogp_parser_t g_pair_new_parser;
static btstack_timer_source_t g_pair_new_timer;
static bool g_pair_new_timer_active;
static atomic_bool g_pair_new_request = ATOMIC_VAR_INIT(false);
static atomic_bool g_pair_new_cancel = ATOMIC_VAR_INIT(false);

static blu2usb_ble_hogp_vendor_backend_t g_vendor_backend;
static bool g_vendor_registered;

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel,
                                     uint8_t *packet, uint16_t size)
{
    (void)packet_type; (void)channel; (void)size;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META) return;

    switch (hci_event_gattservice_meta_get_subevent_code(packet)) {
    case GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED: {
        const uint16_t cid =
            gattservice_subevent_hid_service_connected_get_hids_cid(packet);
        const uint8_t status =
            gattservice_subevent_hid_service_connected_get_status(packet);
        const bool candidate =
            g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid;

        if (candidate) {
            if (status != ERROR_CODE_SUCCESS) {
                pair_new_disconnect_candidate(true, true);
                return;
            }

            const uint8_t *descriptor =
                hids_client_descriptor_storage_get_descriptor_data(cid, 0u);
            const uint16_t descriptor_len =
                hids_client_descriptor_storage_get_descriptor_len(cid, 0u);
            const blu2usb_hid_source_t source =
                blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE, 1u);

            const bool parser_ready =
                descriptor != NULL && descriptor_len > 0u &&
                blu2usb_ble_hogp_parser_configure(
                    &g_pair_new_parser, source, descriptor, descriptor_len) &&
                blu2usb_ble_hogp_parser_has_mouse(&g_pair_new_parser);

            if (!parser_ready) {
                if (descriptor != NULL && descriptor_len > 0u)
                    reject_address(g_pair_new_address, g_pair_new_address_type);
                pair_new_disconnect_candidate(true, true);
                return;
            }

            /* A saved candidate must never satisfy Pair New. If pairing did
             * not create a new bond, keep the original 15-second window. */
            if (!pair_new_created_new_bond()) {
                pair_new_disconnect_candidate(false, true);
                return;
            }

            g_pair_new_state = BLE_PAIR_NEW_READY;
            pair_new_begin_handoff();
            return;
        }

        if (cid != g_hids_cid) return;
        if (status != ERROR_CODE_SUCCESS) {
            disconnect_and_rescan();
            return;
        }

        const uint8_t *descriptor =
            hids_client_descriptor_storage_get_descriptor_data(g_hids_cid, 0u);
        const uint16_t descriptor_len =
            hids_client_descriptor_storage_get_descriptor_len(g_hids_cid, 0u);
        const blu2usb_hid_source_t source =
            blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE, 1u);
        const bool parser_ready =
            descriptor != NULL && descriptor_len > 0u &&
            blu2usb_ble_hogp_parser_configure(
                &g_parser, source, descriptor, descriptor_len) &&
            blu2usb_ble_hogp_parser_has_mouse(&g_parser);

        if (!parser_ready) {
            if (descriptor != NULL && descriptor_len > 0u)
                reject_address(g_remote_address, g_remote_address_type);
            disconnect_and_rescan();
            return;
        }

        g_state = BLE_HOGP_STATE_READY;
        stop_reconnect_timer();
        g_saved_search_active = false;
        g_reconnect_after_disconnect = false;
        if (g_vendor_registered)
            g_vendor_backend.session(g_vendor_backend.context, true);
        if (!publish_status(BLU2USB_BLE_HOGP_MESSAGE_CONNECTED)) {
            disconnect_and_rescan();
            return;
        }
        service_vendor_output();
        break;
    }

    case GATTSERVICE_SUBEVENT_HID_SERVICE_DISCONNECTED: {
        const uint16_t cid =
            gattservice_subevent_hid_service_disconnected_get_hids_cid(packet);

        if (g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid) {
            /* HCI disconnection owns candidate cleanup/resume. */
            break;
        }

        if (cid == g_hids_cid) {
            if (g_pair_new_handoff_pending) break;
            if (g_state != BLE_HOGP_STATE_DISCONNECTING)
                disconnect_current(g_state == BLE_HOGP_STATE_READY);
        }
        break;
    }

    case GATTSERVICE_SUBEVENT_HID_REPORT: {
        const uint16_t cid =
            gattservice_subevent_hid_report_get_hids_cid(packet);

        if (g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid) {
            /* Candidate input is never authoritative before promotion. */
            break;
        }

        if (g_state != BLE_HOGP_STATE_READY || cid != g_hids_cid) break;

        const uint8_t report_id =
            gattservice_subevent_hid_report_get_report_id(packet);
        const uint8_t *raw =
            gattservice_subevent_hid_report_get_report(packet);
        const uint16_t raw_len =
            gattservice_subevent_hid_report_get_report_len(packet);
        const uint8_t *payload = NULL;
        size_t payload_len = 0u;

        if (!blu2usb_ble_hogp_parser_normalize_report(
                &g_parser, report_id, raw, raw_len, &payload, &payload_len)) {
            disconnect_and_rescan();
            break;
        }

        const blu2usb_hid_source_t source =
            blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE, 1u);
        bool consumed = false;
        if (g_vendor_registered)
            consumed = g_vendor_backend.input(
                g_vendor_backend.context, source, report_id, payload,
                payload_len, publish_runtime_mouse_event, NULL);

        if (!consumed &&
            !blu2usb_ble_hogp_parser_parse_report(
                &g_parser, report_id, payload, payload_len,
                publish_mouse_event, NULL)) {
            disconnect_and_rescan();
        }

        service_vendor_output();
        break;
    }

    default:
        break;
    }
}

static void hci_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size)
{
    (void)channel; (void)size;
    if (packet_type != HCI_EVENT_PACKET) return;
    switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING &&
            g_state == BLE_HOGP_STATE_WAITING_FOR_STACK) reconnect_or_scan();
        break;
    case GAP_EVENT_ADVERTISING_REPORT: {
        if (g_state != BLE_HOGP_STATE_SCANNING || !advertisement_has_hid_service(packet)) break;
        bd_addr_t address;
        gap_event_advertising_report_get_address(packet, address);
        const bd_addr_type_t type = gap_event_advertising_report_get_address_type(packet);
        const uint16_t appearance = advertisement_appearance(packet);
        if (address_is_rejected(address, type)) break;
        if (appearance_is_explicit_non_mouse_hid(appearance)) { reject_address(address, type); break; }
        gap_stop_scan();
        stop_reconnect_timer();
        memcpy(g_remote_address, address, sizeof(bd_addr_t));
        g_remote_address_type = type;
        g_reconnect_cancel_pending = false;
        g_state = BLE_HOGP_STATE_CONNECTING;
        if (gap_connect(g_remote_address, g_remote_address_type) != ERROR_CODE_SUCCESS)
            start_scan();
        break;
    }
    case HCI_EVENT_META_GAP:
        if (hci_event_gap_meta_get_subevent_code(packet) == GAP_SUBEVENT_LE_CONNECTION_COMPLETE &&
            g_state == BLE_HOGP_STATE_CONNECTING) {
            const uint8_t status = gap_subevent_le_connection_complete_get_status(packet);
            if (g_reconnect_cancel_pending) {
                g_reconnect_cancel_pending = false;
                if (status == ERROR_CODE_SUCCESS) {
                    g_connection_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
                    g_idle_after_disconnect = true;
                    g_state = BLE_HOGP_STATE_DISCONNECTING;
                    gap_disconnect(g_connection_handle);
                } else {
                    g_connection_handle = HCI_CON_HANDLE_INVALID;
                    g_state = BLE_HOGP_STATE_IDLE;
                }
                break;
            }
            if (status != ERROR_CODE_SUCCESS) {
                g_connection_handle = HCI_CON_HANDLE_INVALID;
                if (g_saved_search_active) {
                    g_saved_search_active = false;
                    stop_reconnect_timer();
                    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_SAVED_SEARCH_TIMEOUT);
                    g_state = BLE_HOGP_STATE_IDLE;
                } else {
                    start_scan();
                }
                break;
            }
            if (!g_saved_search_active) stop_reconnect_timer();
            g_connection_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
            g_state = BLE_HOGP_STATE_SECURING;
            sm_request_pairing(g_connection_handle);
        }
        break;
    case HCI_EVENT_DISCONNECTION_COMPLETE: {
        if (g_idle_after_disconnect) {
            g_idle_after_disconnect = false;
            g_connection_handle = HCI_CON_HANDLE_INVALID;
            g_hids_cid = 0u;
            memset(&g_parser, 0, sizeof(g_parser));
            g_state = BLE_HOGP_STATE_IDLE;
            break;
        }
        const bool was_ready = g_state == BLE_HOGP_STATE_READY;
        const bool reconnect_bonded = was_ready || g_reconnect_after_disconnect;
        stop_reconnect_timer();
        if (was_ready && g_vendor_registered) g_vendor_backend.session(g_vendor_backend.context, false);
        g_connection_handle = HCI_CON_HANDLE_INVALID;
        g_hids_cid = 0u;
        memset(&g_parser, 0, sizeof(g_parser));
        if (was_ready) (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_DISCONNECTED);
        g_reconnect_after_disconnect = false;
        if (reconnect_bonded) reconnect_or_scan();
        else start_scan();
        break;
    }
    default: break;
    }
}

static void sm_packet_handler(uint8_t packet_type, uint16_t channel,
                              uint8_t *packet, uint16_t size)
{
    (void)channel; (void)size;
    if (packet_type != HCI_EVENT_PACKET) return;
    bool ready = false;
    switch (hci_event_packet_get_type(packet)) {
    case SM_EVENT_JUST_WORKS_REQUEST:
        sm_just_works_confirm(sm_event_just_works_request_get_handle(packet)); break;
    case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
        sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet)); break;
    case SM_EVENT_PAIRING_COMPLETE:
        if (sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS) ready = true;
        else disconnect_and_rescan();
        break;
    case SM_EVENT_REENCRYPTION_COMPLETE:
        if (sm_event_reencryption_complete_get_status(packet) == ERROR_CODE_SUCCESS) ready = true;
        else disconnect_and_rescan();
        break;
    default: break;
    }
    if (ready && g_state == BLE_HOGP_STATE_SECURING) connect_hid_service();
}

static void ble_hogp_session_setup(void)
{
    memset(&g_parser, 0, sizeof(g_parser));
    memset(g_rejected_devices, 0, sizeof(g_rejected_devices));
    memset(g_remote_address, 0, sizeof(g_remote_address));
    g_remote_address_type = BD_ADDR_TYPE_UNKNOWN;
    g_rejected_next = 0u;
    g_state = BLE_HOGP_STATE_WAITING_FOR_STACK;
    g_connection_handle = HCI_CON_HANDLE_INVALID;
    g_hids_cid = 0u;
    g_reconnect_timer_active = false;
    g_reconnect_cancel_pending = false;
    g_reconnect_after_disconnect = false;
    g_idle_after_disconnect = false;
    g_saved_search_active = false;
    atomic_store_explicit(&g_saved_search_request, false, memory_order_relaxed);
    atomic_store_explicit(&g_saved_search_cancel, false, memory_order_relaxed);
    hids_client_init(g_descriptor_storage, sizeof(g_descriptor_storage));
    g_hci_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&g_hci_registration);
    g_sm_registration.callback = &sm_packet_handler;
    sm_add_event_handler(&g_sm_registration);
    btstack_run_loop_set_timer_handler(&g_reconnect_timer, reconnect_timeout_handler);
    btstack_run_loop_set_timer_handler(&g_vendor_timer, vendor_timer_handler);
    btstack_run_loop_set_timer(&g_vendor_timer, BLE_HOGP_VENDOR_SERVICE_MS);
    btstack_run_loop_add_timer(&g_vendor_timer);
}

bool blu2usb_ble_hogp_start(void)
{
    return blu2usb_bt_runtime_start(ble_hogp_session_setup);
}

unsigned blu2usb_ble_hogp_pico_bonded_mouse_count(void)
{
    const int count = le_device_db_count();
    return count > 0 ? (unsigned)count : 0u;
}

void blu2usb_ble_hogp_pico_request_saved_search(void)
{
    atomic_store_explicit(&g_saved_search_request, true, memory_order_release);
}

void blu2usb_ble_hogp_pico_cancel_saved_search(void)
{
    atomic_store_explicit(&g_saved_search_cancel, true, memory_order_release);
}
