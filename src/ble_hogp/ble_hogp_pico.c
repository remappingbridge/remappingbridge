#include "blu2usb/ble_hogp/ble_hogp.h"

#include <stdatomic.h>
#include <string.h>
#include "btstack.h"
#include "btstack_tlv.h"
#include "ble/le_device_db.h"

#define BLE_HOGP_DESCRIPTOR_STORAGE_SIZE 4096u
#define BLE_HOGP_REJECTED_DEVICE_CAPACITY 4u
#define BLE_APPEARANCE_HID_GENERIC 960u
#define BLE_APPEARANCE_HID_MOUSE 962u
#define BLE_APPEARANCE_HID_LAST 1023u
#define BLE_HOGP_VENDOR_SERVICE_MS 20u
#define BLE_HOGP_BONDED_RECONNECT_TIMEOUT_MS 8000u
#define BLE_HOGP_PAIR_NEW_TIMEOUT_MS 15000u
#define BLE_HOGP_MOUSE_NAME_CAPACITY 64u
#define BLE_HOGP_SAVED_NAME_CAPACITY 32u
#define BLE_HOGP_SAVED_REGISTRY_CAPACITY 8u
#define BLE_HOGP_SAVED_REGISTRY_VERSION 2u
#define BLE_HOGP_SAVED_REGISTRY_VERSION_V1 1u
#define BLE_HOGP_SAVED_REGISTRY_TAG UINT32_C(0x4232534e) /* B2SN */
#define BLE_HOGP_SAVED_REGISTRY_V1_ENTRY_SIZE \
    (1u + 1u + 6u + BLE_HOGP_SAVED_NAME_CAPACITY)
#define BLE_HOGP_SAVED_REGISTRY_ENTRY_SIZE \
    (1u + 1u + 6u + 16u + BLE_HOGP_SAVED_NAME_CAPACITY)
#define BLE_HOGP_SAVED_REGISTRY_V1_SERIALIZED_SIZE \
    (1u + BLE_HOGP_SAVED_REGISTRY_CAPACITY * BLE_HOGP_SAVED_REGISTRY_V1_ENTRY_SIZE)
#define BLE_HOGP_SAVED_REGISTRY_SERIALIZED_SIZE \
    (1u + BLE_HOGP_SAVED_REGISTRY_CAPACITY * BLE_HOGP_SAVED_REGISTRY_ENTRY_SIZE)

_Static_assert(BLE_HOGP_SAVED_REGISTRY_CAPACITY == NVM_NUM_DEVICE_DB_ENTRIES,
               "saved-name registry must cover the LE Device DB");

_Static_assert(sizeof(blu2usb_canonical_mouse_event_t) <= BLU2USB_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE,
               "canonical mouse event must fit runtime message");

typedef enum {
    BLE_HOGP_STATE_WAITING_FOR_STACK = 0,
    BLE_HOGP_STATE_IDLE,
    BLE_HOGP_STATE_SCANNING,
    BLE_HOGP_STATE_CONNECTING,
    BLE_HOGP_STATE_SECURING,
    BLE_HOGP_STATE_READING_NAME,
    BLE_HOGP_STATE_CONNECTING_HIDS,
    BLE_HOGP_STATE_READY,
    BLE_HOGP_STATE_DISCONNECTING,
} ble_hogp_state_t;

typedef enum {
    BLE_PAIR_NEW_IDLE = 0,
    BLE_PAIR_NEW_SCANNING,
    BLE_PAIR_NEW_CONNECTING,
    BLE_PAIR_NEW_SECURING,
    BLE_PAIR_NEW_READING_NAME,
    BLE_PAIR_NEW_CONNECTING_HIDS,
    BLE_PAIR_NEW_READY,
    BLE_PAIR_NEW_DISCONNECTING,
} ble_pair_new_state_t;

typedef struct {
    bool used;
    bd_addr_type_t address_type;
    bd_addr_t address;
} ble_hogp_rejected_device_t;

typedef struct {
    bool used;
    bd_addr_type_t address_type;
    bd_addr_t address;
    sm_key_t irk;
    char name[BLE_HOGP_SAVED_NAME_CAPACITY];
} ble_hogp_saved_name_t;

static ble_hogp_state_t g_state;
static bd_addr_t g_remote_address;
static bd_addr_type_t g_remote_address_type;
static hci_con_handle_t g_connection_handle = HCI_CON_HANDLE_INVALID;
static int g_current_bond_index = -1;
static uint16_t g_hids_cid;
static char g_current_mouse_name[BLE_HOGP_MOUSE_NAME_CAPACITY];
static uint8_t g_descriptor_storage[BLE_HOGP_DESCRIPTOR_STORAGE_SIZE];
static blu2usb_ble_hogp_parser_t g_parser;
static ble_hogp_rejected_device_t g_rejected_devices[BLE_HOGP_REJECTED_DEVICE_CAPACITY];
static size_t g_rejected_next;
static ble_hogp_saved_name_t g_saved_names[BLE_HOGP_SAVED_REGISTRY_CAPACITY];
static bool g_saved_names_loaded;
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
static int g_pair_new_bond_index = -1;
static bd_addr_t g_pair_new_address;
static bd_addr_type_t g_pair_new_address_type;
static hci_con_handle_t g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
static uint16_t g_pair_new_hids_cid;
static char g_pair_new_mouse_name[BLE_HOGP_MOUSE_NAME_CAPACITY];
static blu2usb_ble_hogp_parser_t g_pair_new_parser;
static btstack_timer_source_t g_pair_new_timer;
static bool g_pair_new_timer_active;
static atomic_bool g_pair_new_request = ATOMIC_VAR_INIT(false);
static atomic_bool g_pair_new_cancel = ATOMIC_VAR_INIT(false);

static blu2usb_ble_hogp_vendor_backend_t g_vendor_backend;
static bool g_vendor_registered;

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel,
                                     uint8_t *packet, uint16_t size);
static void handle_name_gatt_event(uint8_t packet_type, uint16_t channel,
                                   uint8_t *packet, uint16_t size);
static void start_scan(void);
static void reconnect_or_scan(void);
static void pair_new_resume_scan(void);
static void pair_new_finalize_promotion(void);
static void connect_hid_service(void);
static void pair_new_connect_hid_service(void);
static void read_current_mouse_name(void);
static void read_pair_new_mouse_name(void);
static void service_vendor_output(void);
static void saved_names_load(void);
static void saved_names_remember_bond(int bond_index, const char *name);
static int resolve_current_bond_index(void);

static bool bond_slot_info(int slot,
                           bd_addr_type_t *address_type,
                           bd_addr_t address,
                           sm_key_t irk)
{
    if (slot < 0 || slot >= le_device_db_max_count()) return false;

    int saved_type = (int)BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t saved_address;
    sm_key_t saved_irk;
    memset(saved_address, 0, sizeof(saved_address));
    memset(saved_irk, 0, sizeof(saved_irk));
    le_device_db_info(slot, &saved_type, saved_address, saved_irk);
    if (saved_type == (int)BD_ADDR_TYPE_UNKNOWN) return false;

    if (address_type != NULL) *address_type = (bd_addr_type_t)saved_type;
    if (address != NULL) memcpy(address, saved_address, sizeof(bd_addr_t));
    if (irk != NULL) memcpy(irk, saved_irk, sizeof(sm_key_t));
    return true;
}

static bool irk_is_nonzero(const sm_key_t irk)
{
    if (irk == NULL) return false;
    for (unsigned index = 0u; index < sizeof(sm_key_t); ++index)
        if (irk[index] != 0u) return true;
    return false;
}

static bool bond_identity_equal(bd_addr_type_t left_type,
                                const bd_addr_t left_address,
                                const sm_key_t left_irk,
                                bd_addr_type_t right_type,
                                const bd_addr_t right_address,
                                const sm_key_t right_irk)
{
    if (irk_is_nonzero(left_irk) && irk_is_nonzero(right_irk) &&
        memcmp(left_irk, right_irk, sizeof(sm_key_t)) == 0)
        return true;
    return left_type == right_type &&
        memcmp(left_address, right_address, sizeof(bd_addr_t)) == 0;
}

static bool bond_slot_is_first_for_identity(int slot)
{
    bd_addr_type_t type = BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t address;
    sm_key_t irk;
    if (!bond_slot_info(slot, &type, address, irk)) return false;

    for (int prior = 0; prior < slot; ++prior) {
        bd_addr_type_t prior_type = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t prior_address;
        sm_key_t prior_irk;
        if (!bond_slot_info(prior, &prior_type, prior_address, prior_irk)) continue;
        if (bond_identity_equal(type, address, irk,
                                prior_type, prior_address, prior_irk))
            return false;
    }
    return true;
}

static int bond_slot_for_ordinal(int ordinal)
{
    if (ordinal < 0) return -1;
    int seen = 0;
    for (int slot = 0; slot < le_device_db_max_count(); ++slot) {
        if (!bond_slot_is_first_for_identity(slot)) continue;
        if (seen == ordinal) return slot;
        ++seen;
    }
    return -1;
}

static int bond_unique_count(void)
{
    int count = 0;
    for (int slot = 0; slot < le_device_db_max_count(); ++slot)
        if (bond_slot_is_first_for_identity(slot)) ++count;
    return count;
}

static int bond_ordinal_for_slot(int wanted_slot)
{
    bd_addr_type_t wanted_type = BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t wanted_address;
    sm_key_t wanted_irk;
    if (!bond_slot_info(wanted_slot, &wanted_type, wanted_address, wanted_irk))
        return -1;

    int ordinal = 0;
    for (int slot = 0; slot < le_device_db_max_count(); ++slot) {
        if (!bond_slot_is_first_for_identity(slot)) continue;

        bd_addr_type_t type = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t address;
        sm_key_t irk;
        if (!bond_slot_info(slot, &type, address, irk)) continue;
        if (bond_identity_equal(wanted_type, wanted_address, wanted_irk,
                                type, address, irk))
            return ordinal;
        ++ordinal;
    }
    return -1;
}

static bool saved_identity_for_bond(int bond_slot,
                                    bd_addr_type_t *address_type,
                                    bd_addr_t address)
{
    return address_type != NULL && address != NULL &&
        bond_slot_info(bond_slot, address_type, address, NULL);
}

static int saved_names_find(bd_addr_type_t address_type,
                            const bd_addr_t address)
{
    for (unsigned index = 0u; index < BLE_HOGP_SAVED_REGISTRY_CAPACITY; ++index) {
        if (!g_saved_names[index].used) continue;
        if (g_saved_names[index].address_type == address_type &&
            memcmp(g_saved_names[index].address, address, sizeof(bd_addr_t)) == 0)
            return (int)index;
    }
    return -1;
}

static int saved_names_free_slot(void)
{
    for (unsigned index = 0u; index < BLE_HOGP_SAVED_REGISTRY_CAPACITY; ++index)
        if (!g_saved_names[index].used) return (int)index;
    return -1;
}

static bool saved_names_store(void)
{
    const btstack_tlv_t *tlv = NULL;
    void *context = NULL;
    btstack_tlv_get_instance(&tlv, &context);
    if (tlv == NULL || context == NULL) return false;

    uint8_t payload[BLE_HOGP_SAVED_REGISTRY_SERIALIZED_SIZE];
    memset(payload, 0, sizeof(payload));
    payload[0] = BLE_HOGP_SAVED_REGISTRY_VERSION;
    size_t offset = 1u;
    for (unsigned index = 0u; index < BLE_HOGP_SAVED_REGISTRY_CAPACITY; ++index) {
        const ble_hogp_saved_name_t *entry = &g_saved_names[index];
        payload[offset++] = entry->used ? 1u : 0u;
        payload[offset++] = (uint8_t)entry->address_type;
        memcpy(&payload[offset], entry->address, sizeof(bd_addr_t));
        offset += sizeof(bd_addr_t);
        memcpy(&payload[offset], entry->name, BLE_HOGP_SAVED_NAME_CAPACITY);
        offset += BLE_HOGP_SAVED_NAME_CAPACITY;
    }
    return tlv->store_tag(context, BLE_HOGP_SAVED_REGISTRY_TAG,
                          payload, sizeof(payload)) == 0;
}

static void saved_names_load(void)
{
    if (g_saved_names_loaded) return;
    g_saved_names_loaded = true;
    memset(g_saved_names, 0, sizeof(g_saved_names));

    const btstack_tlv_t *tlv = NULL;
    void *context = NULL;
    btstack_tlv_get_instance(&tlv, &context);
    if (tlv == NULL || context == NULL) return;

    uint8_t payload[BLE_HOGP_SAVED_REGISTRY_SERIALIZED_SIZE];
    const int length = tlv->get_tag(context, BLE_HOGP_SAVED_REGISTRY_TAG,
                                    payload, sizeof(payload));
    if (length != (int)sizeof(payload) ||
        payload[0] != BLE_HOGP_SAVED_REGISTRY_VERSION) return;

    size_t offset = 1u;
    for (unsigned index = 0u; index < BLE_HOGP_SAVED_REGISTRY_CAPACITY; ++index) {
        ble_hogp_saved_name_t *entry = &g_saved_names[index];
        entry->used = payload[offset++] != 0u;
        entry->address_type = (bd_addr_type_t)payload[offset++];
        memcpy(entry->address, &payload[offset], sizeof(bd_addr_t));
        offset += sizeof(bd_addr_t);
        memcpy(entry->name, &payload[offset], BLE_HOGP_SAVED_NAME_CAPACITY);
        entry->name[BLE_HOGP_SAVED_NAME_CAPACITY - 1u] = '\0';
        offset += BLE_HOGP_SAVED_NAME_CAPACITY;
        if (!entry->used) {
            memset(entry, 0, sizeof(*entry));
            continue;
        }
    }
}

static void saved_names_remember_bond(int bond_index, const char *name)
{
    if (name == NULL || name[0] == '\0') return;
    saved_names_load();

    bd_addr_type_t address_type = BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t address;
    if (!saved_identity_for_bond(bond_index, &address_type, address)) return;

    int slot = saved_names_find(address_type, address);
    if (slot < 0) slot = saved_names_free_slot();
    if (slot < 0) return;

    ble_hogp_saved_name_t candidate = g_saved_names[slot];
    memset(&candidate, 0, sizeof(candidate));
    candidate.used = true;
    candidate.address_type = address_type;
    memcpy(candidate.address, address, sizeof(bd_addr_t));
    size_t length = 0u;
    while (name[length] != '\0' &&
           length + 1u < BLE_HOGP_SAVED_NAME_CAPACITY) {
        candidate.name[length] = name[length];
        ++length;
    }
    candidate.name[length] = '\0';

    if (memcmp(&candidate, &g_saved_names[slot], sizeof(candidate)) == 0) return;
    g_saved_names[slot] = candidate;
    (void)saved_names_store();
}

static int resolve_current_bond_index(void)
{
    const int count = le_device_db_count();
    if (g_state != BLE_HOGP_STATE_READY ||
        g_connection_handle == HCI_CON_HANDLE_INVALID || count <= 0) return -1;

    if (bond_slot_info(g_current_bond_index, NULL, NULL, NULL))
        return g_current_bond_index;

    const int security_manager_index = sm_le_device_index(g_connection_handle);
    if (bond_slot_info(security_manager_index, NULL, NULL, NULL)) {
        g_current_bond_index = security_manager_index;
        return security_manager_index;
    }

    if (count == 1) {
        g_current_bond_index = bond_slot_for_ordinal(0);
        return g_current_bond_index;
    }
    return -1;
}

bool blu2usb_ble_hogp_register_vendor_backend(
    const blu2usb_ble_hogp_vendor_backend_t *backend)
{
    if (backend == NULL || g_vendor_registered || backend->input == NULL ||
        backend->next_output == NULL || backend->output_result == NULL ||
        backend->claims_button == NULL || backend->session == NULL) return false;
    g_vendor_backend = *backend;
    g_vendor_registered = true;
    return true;
}

static bool publish_status(blu2usb_ble_hogp_message_type_t type)
{
    return blu2usb_bt_runtime_publish(BLU2USB_BLE_HOGP_RUNTIME_CHANNEL,
                                       (uint16_t)type, NULL, 0u);
}

static bool publish_runtime_mouse_event(void *context,
                                        const blu2usb_canonical_mouse_event_t *event)
{
    (void)context;
    return event != NULL && blu2usb_bt_runtime_publish(
        BLU2USB_BLE_HOGP_RUNTIME_CHANNEL, BLU2USB_BLE_HOGP_MESSAGE_MOUSE,
        event, (uint16_t)sizeof(*event));
}

static bool publish_mouse_event(void *context,
                                const blu2usb_canonical_mouse_event_t *event)
{
    (void)context;
    if (event != NULL && event->type == BLU2USB_MOUSE_EVENT_BUTTON &&
        g_vendor_registered && g_vendor_backend.claims_button(
            g_vendor_backend.context, event->data.button.button)) {
        return true;
    }
    return publish_runtime_mouse_event(NULL, event);
}

static bool advertisement_has_hid_service(const uint8_t *packet)
{
    const uint8_t *data = gap_event_advertising_report_get_data(packet);
    const uint8_t length = gap_event_advertising_report_get_data_length(packet);
    return ad_data_contains_uuid16(length, data,
        ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE);
}

static uint16_t advertisement_appearance(const uint8_t *packet)
{
    const uint8_t *data = gap_event_advertising_report_get_data(packet);
    const uint8_t length = gap_event_advertising_report_get_data_length(packet);
    ad_context_t context;
    for (ad_iterator_init(&context, length, (uint8_t *)data);
         ad_iterator_has_more(&context); ad_iterator_next(&context)) {
        if (ad_iterator_get_data_type(&context) == BLUETOOTH_DATA_TYPE_APPEARANCE &&
            ad_iterator_get_data_len(&context) >= 2u)
            return little_endian_read_16(ad_iterator_get_data(&context), 0u);
    }
    return 0u;
}

static bool appearance_is_explicit_non_mouse_hid(uint16_t appearance)
{
    return appearance >= BLE_APPEARANCE_HID_GENERIC &&
        appearance <= BLE_APPEARANCE_HID_LAST &&
        appearance != BLE_APPEARANCE_HID_GENERIC &&
        appearance != BLE_APPEARANCE_HID_MOUSE;
}

static bool address_is_rejected(const bd_addr_t address, bd_addr_type_t type)
{
    for (size_t i = 0u; i < BLE_HOGP_REJECTED_DEVICE_CAPACITY; ++i)
        if (g_rejected_devices[i].used && g_rejected_devices[i].address_type == type &&
            memcmp(g_rejected_devices[i].address, address, sizeof(bd_addr_t)) == 0) return true;
    return false;
}

static void reject_address(const bd_addr_t address, bd_addr_type_t type)
{
    if (address_is_rejected(address, type)) return;
    ble_hogp_rejected_device_t *slot = &g_rejected_devices[g_rejected_next];
    slot->used = true;
    slot->address_type = type;
    memcpy(slot->address, address, sizeof(bd_addr_t));
    g_rejected_next = (g_rejected_next + 1u) % BLE_HOGP_REJECTED_DEVICE_CAPACITY;
}

static bool address_is_saved(const bd_addr_t address, bd_addr_type_t type)
{
    for (int slot = 0; slot < le_device_db_max_count(); ++slot) {
        bd_addr_type_t saved_type = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t saved_address;
        if (!bond_slot_info(slot, &saved_type, saved_address, NULL)) continue;
        if (saved_type == type &&
            memcmp(saved_address, address, sizeof(bd_addr_t)) == 0) {
            return true;
        }
    }
    return false;
}

static void remove_pair_new_bond(void)
{
    if (bond_slot_info(g_pair_new_bond_index, NULL, NULL, NULL)) {
        le_device_db_remove(g_pair_new_bond_index);
        g_pair_new_bond_index = -1;
        return;
    }

    for (int slot = 0; slot < le_device_db_max_count(); ++slot) {
        bd_addr_type_t saved_type = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t saved_address;
        if (!bond_slot_info(slot, &saved_type, saved_address, NULL)) continue;
        if (saved_type == g_pair_new_address_type &&
            memcmp(saved_address, g_pair_new_address, sizeof(bd_addr_t)) == 0) {
            le_device_db_remove(slot);
            break;
        }
    }
}

static bool pair_new_created_new_bond(void)
{
    return le_device_db_count() > g_pair_new_bond_count_before;
}

static void stop_pair_new_timer(void)
{
    if (!g_pair_new_timer_active) return;
    (void)btstack_run_loop_remove_timer(&g_pair_new_timer);
    g_pair_new_timer_active = false;
}

static void pair_new_clear_candidate(void)
{
    memset(g_pair_new_address, 0, sizeof(g_pair_new_address));
    g_pair_new_address_type = BD_ADDR_TYPE_UNKNOWN;
    g_pair_new_mouse_name[0] = '\0';
    g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
    g_pair_new_bond_index = -1;
    g_pair_new_hids_cid = 0u;
    g_pair_new_mouse_name[0] = '\0';
    memset(&g_pair_new_parser, 0, sizeof(g_pair_new_parser));
    g_pair_new_cancel_pending = false;
    g_pair_new_resume_after_disconnect = false;
    if (!g_pair_new_handoff_pending)
        g_pair_new_state = BLE_PAIR_NEW_IDLE;
}

static void pair_new_resume_scan(void)
{
    if (!g_pair_new_active || !g_pair_new_timer_active) {
        if (!g_pair_new_handoff_pending)
            g_pair_new_state = BLE_PAIR_NEW_IDLE;
        return;
    }

    pair_new_clear_candidate();
    g_pair_new_state = BLE_PAIR_NEW_SCANNING;
    gap_set_scan_parameters(0u, 48u, 48u);
    gap_start_scan();
}

static void pair_new_disconnect_candidate(bool remove_bond, bool resume)
{
    if (remove_bond)
        remove_pair_new_bond();

    g_pair_new_resume_after_disconnect = resume;
    if (g_pair_new_connection_handle != HCI_CON_HANDLE_INVALID) {
        g_pair_new_state = BLE_PAIR_NEW_DISCONNECTING;
        gap_disconnect(g_pair_new_connection_handle);
    } else {
        pair_new_clear_candidate();
        if (resume)
            pair_new_resume_scan();
    }
}

static void stop_reconnect_timer(void)
{
    if (!g_reconnect_timer_active) return;
    (void)btstack_run_loop_remove_timer(&g_reconnect_timer);
    g_reconnect_timer_active = false;
}

static void start_scan(void)
{
    stop_reconnect_timer();
    g_saved_search_active = false;
    g_reconnect_cancel_pending = false;
    g_current_bond_index = -1;
    g_state = BLE_HOGP_STATE_SCANNING;
    gap_set_scan_parameters(0u, 48u, 48u);
    gap_start_scan();
}

static void reconnect_timeout_handler(btstack_timer_source_t *timer)
{
    (void)timer;
    g_reconnect_timer_active = false;
    if (!g_saved_search_active) return;

    g_saved_search_active = false;
    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_SAVED_SEARCH_TIMEOUT);

    if (g_state == BLE_HOGP_STATE_CONNECTING) {
        g_reconnect_cancel_pending = true;
        if (gap_connect_cancel() != ERROR_CODE_SUCCESS) {
            g_reconnect_cancel_pending = false;
            g_state = BLE_HOGP_STATE_IDLE;
        }
    } else if (g_connection_handle != HCI_CON_HANDLE_INVALID) {
        g_idle_after_disconnect = true;
        g_state = BLE_HOGP_STATE_DISCONNECTING;
        gap_disconnect(g_connection_handle);
    } else {
        g_state = BLE_HOGP_STATE_IDLE;
    }
}

static void pair_new_timeout_handler(btstack_timer_source_t *timer)
{
    (void)timer;
    g_pair_new_timer_active = false;
    if (!g_pair_new_active) return;

    g_pair_new_active = false;
    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_TIMEOUT);

    if (g_pair_new_state == BLE_PAIR_NEW_SCANNING) {
        gap_stop_scan();
        pair_new_clear_candidate();
        return;
    }

    if (g_pair_new_state == BLE_PAIR_NEW_CONNECTING) {
        g_pair_new_cancel_pending = true;
        if (gap_connect_cancel() != ERROR_CODE_SUCCESS) {
            g_pair_new_cancel_pending = false;
            pair_new_clear_candidate();
        }
        return;
    }

    if (g_pair_new_connection_handle != HCI_CON_HANDLE_INVALID)
        pair_new_disconnect_candidate(true, false);
    else
        pair_new_clear_candidate();
}

static bool start_bonded_reconnect(void)
{
    if (le_device_db_count() <= 0) return false;

    stop_reconnect_timer();
    g_reconnect_cancel_pending = false;
    g_current_bond_index = -1;
    (void)gap_whitelist_clear();
    (void)gap_load_resolving_list_from_le_device_db();

    unsigned added = 0u;
    for (int slot = 0; slot < le_device_db_max_count(); ++slot) {
        bd_addr_type_t address_type = BD_ADDR_TYPE_UNKNOWN;
        bd_addr_t address;
        if (!bond_slot_info(slot, &address_type, address, NULL)) continue;
        if (gap_whitelist_add(address_type, address) != ERROR_CODE_SUCCESS)
            continue;
        if (added == 0u) {
            memcpy(g_remote_address, address, sizeof(bd_addr_t));
            g_remote_address_type = address_type;
        }
        ++added;
    }

    if (added == 0u || gap_connect_with_whitelist() != ERROR_CODE_SUCCESS) return false;

    g_state = BLE_HOGP_STATE_CONNECTING;
    g_saved_search_active = true;
    btstack_run_loop_set_timer(&g_reconnect_timer,
                               BLE_HOGP_BONDED_RECONNECT_TIMEOUT_MS);
    btstack_run_loop_add_timer(&g_reconnect_timer);
    g_reconnect_timer_active = true;
    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_SAVED_SEARCH_STARTED);
    return true;
}

static void reconnect_or_scan(void)
{
    if (!start_bonded_reconnect()) start_scan();
}

static bool start_pair_new(void)
{
    if (g_pair_new_active || g_pair_new_handoff_pending) return false;

    if (g_saved_search_active || g_reconnect_cancel_pending ||
        (g_state != BLE_HOGP_STATE_READY &&
         (g_state == BLE_HOGP_STATE_CONNECTING ||
          g_state == BLE_HOGP_STATE_SECURING ||
          g_state == BLE_HOGP_STATE_CONNECTING_HIDS ||
          g_state == BLE_HOGP_STATE_DISCONNECTING))) {
        return false;
    }

    if (g_state == BLE_HOGP_STATE_SCANNING) {
        gap_stop_scan();
        g_state = BLE_HOGP_STATE_IDLE;
    }

    pair_new_clear_candidate();
    g_pair_new_active = true;
    g_pair_new_state = BLE_PAIR_NEW_SCANNING;
    g_pair_new_bond_count_before = le_device_db_count();
    g_pair_new_bond_index = -1;

    btstack_run_loop_set_timer(&g_pair_new_timer, BLE_HOGP_PAIR_NEW_TIMEOUT_MS);
    btstack_run_loop_add_timer(&g_pair_new_timer);
    g_pair_new_timer_active = true;

    gap_set_scan_parameters(0u, 48u, 48u);
    gap_start_scan();
    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_STARTED);
    return true;
}

static void cancel_pair_new(void)
{
    if (!g_pair_new_active && !g_pair_new_handoff_pending) return;

    stop_pair_new_timer();
    g_pair_new_active = false;

    if (g_pair_new_state == BLE_PAIR_NEW_SCANNING) {
        gap_stop_scan();
        pair_new_clear_candidate();
        return;
    }

    if (g_pair_new_state == BLE_PAIR_NEW_CONNECTING) {
        g_pair_new_cancel_pending = true;
        if (gap_connect_cancel() != ERROR_CODE_SUCCESS) {
            g_pair_new_cancel_pending = false;
            pair_new_clear_candidate();
        }
        return;
    }

    if (!g_pair_new_handoff_pending &&
        g_pair_new_connection_handle != HCI_CON_HANDLE_INVALID) {
        pair_new_disconnect_candidate(true, false);
    }
}

static void disconnect_current(bool reconnect_bonded)
{
    const bool was_ready = g_state == BLE_HOGP_STATE_READY;
    if (was_ready && g_vendor_registered)
        g_vendor_backend.session(g_vendor_backend.context, false);
    g_reconnect_after_disconnect = reconnect_bonded;
    g_state = BLE_HOGP_STATE_DISCONNECTING;
    if (was_ready) (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_DISCONNECTED);
    if (g_connection_handle != HCI_CON_HANDLE_INVALID) {
        gap_disconnect(g_connection_handle);
    } else {
        g_reconnect_after_disconnect = false;
        if (reconnect_bonded) reconnect_or_scan();
        else start_scan();
    }
}

static void disconnect_and_rescan(void)
{
    /* During HOPE-08 saved-only search, failures stay inside the bonded
     * reconnect path instead of opening discovery to unsaved devices. */
    disconnect_current(g_saved_search_active);
}

static void copy_mouse_name(char out[BLE_HOGP_MOUSE_NAME_CAPACITY],
                            const uint8_t *value, uint16_t value_len)
{
    size_t length = 0u;
    if (value != NULL) {
        while (length < value_len &&
               length + 1u < BLE_HOGP_MOUSE_NAME_CAPACITY &&
               value[length] != 0u) {
            out[length] = (char)value[length];
            ++length;
        }
    }
    out[length] = '\0';
}

static void handle_name_gatt_event(uint8_t packet_type, uint16_t channel,
                                   uint8_t *packet, uint16_t size)
{
    (void)packet_type;
    (void)channel;
    (void)size;

    switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT: {
        const hci_con_handle_t handle =
            gatt_event_characteristic_value_query_result_get_handle(packet);
        const uint8_t *value =
            gatt_event_characteristic_value_query_result_get_value(packet);
        const uint16_t value_len =
            gatt_event_characteristic_value_query_result_get_value_length(packet);

        if (handle == g_pair_new_connection_handle &&
            g_pair_new_state == BLE_PAIR_NEW_READING_NAME) {
            copy_mouse_name(g_pair_new_mouse_name, value, value_len);
        } else if (handle == g_connection_handle &&
                   g_state == BLE_HOGP_STATE_READING_NAME) {
            copy_mouse_name(g_current_mouse_name, value, value_len);
        }
        break;
    }

    case GATT_EVENT_QUERY_COMPLETE: {
        const hci_con_handle_t handle =
            gatt_event_query_complete_get_handle(packet);

        if (handle == g_pair_new_connection_handle &&
            g_pair_new_state == BLE_PAIR_NEW_READING_NAME) {
            pair_new_connect_hid_service();
        } else if (handle == g_connection_handle &&
                   g_state == BLE_HOGP_STATE_READING_NAME) {
            connect_hid_service();
        }
        break;
    }

    default:
        break;
    }
}

static void read_current_mouse_name(void)
{
    g_current_mouse_name[0] = '\0';
    g_state = BLE_HOGP_STATE_READING_NAME;
    const uint8_t status = gatt_client_read_value_of_characteristics_by_uuid16(
        &handle_name_gatt_event,
        g_connection_handle,
        UINT16_C(0x0001),
        UINT16_C(0xffff),
        ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME);
    if (status != ERROR_CODE_SUCCESS)
        connect_hid_service();
}

static void read_pair_new_mouse_name(void)
{
    g_pair_new_mouse_name[0] = '\0';
    g_pair_new_state = BLE_PAIR_NEW_READING_NAME;
    const uint8_t status = gatt_client_read_value_of_characteristics_by_uuid16(
        &handle_name_gatt_event,
        g_pair_new_connection_handle,
        UINT16_C(0x0001),
        UINT16_C(0xffff),
        ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME);
    if (status != ERROR_CODE_SUCCESS)
        pair_new_connect_hid_service();
}

static void connect_hid_service(void)
{
    g_state = BLE_HOGP_STATE_CONNECTING_HIDS;
    g_hids_cid = 0u;
    const uint8_t status = hids_client_connect(g_connection_handle,
        &handle_gatt_client_event, HID_PROTOCOL_MODE_REPORT, &g_hids_cid);
    if (status != ERROR_CODE_SUCCESS) disconnect_and_rescan();
}

static void pair_new_connect_hid_service(void)
{
    g_pair_new_state = BLE_PAIR_NEW_CONNECTING_HIDS;
    g_pair_new_hids_cid = 0u;
    const uint8_t status = hids_client_connect(g_pair_new_connection_handle,
        &handle_gatt_client_event, HID_PROTOCOL_MODE_REPORT,
        &g_pair_new_hids_cid);
    if (status != ERROR_CODE_SUCCESS)
        pair_new_disconnect_candidate(true, true);
}

static void pair_new_finalize_promotion(void)
{
    int promoted_bond_index = g_pair_new_bond_index;
    if (!bond_slot_info(promoted_bond_index, NULL, NULL, NULL))
        promoted_bond_index = sm_le_device_index(g_pair_new_connection_handle);
    g_current_bond_index =
        bond_slot_info(promoted_bond_index, NULL, NULL, NULL)
            ? promoted_bond_index
            : -1;

    memcpy(g_remote_address, g_pair_new_address, sizeof(bd_addr_t));
    g_remote_address_type = g_pair_new_address_type;
    g_connection_handle = g_pair_new_connection_handle;
    g_hids_cid = g_pair_new_hids_cid;
    g_parser = g_pair_new_parser;
    memcpy(g_current_mouse_name, g_pair_new_mouse_name,
           sizeof(g_current_mouse_name));

    g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
    g_pair_new_bond_index = -1;
    g_pair_new_hids_cid = 0u;
    memset(&g_pair_new_parser, 0, sizeof(g_pair_new_parser));
    g_pair_new_mouse_name[0] = '\0';
    memset(g_pair_new_address, 0, sizeof(g_pair_new_address));
    g_pair_new_address_type = BD_ADDR_TYPE_UNKNOWN;
    g_pair_new_state = BLE_PAIR_NEW_IDLE;
    g_pair_new_active = false;
    g_pair_new_handoff_pending = false;
    g_pair_new_cancel_pending = false;
    g_pair_new_resume_after_disconnect = false;

    g_state = BLE_HOGP_STATE_READY;
    g_reconnect_after_disconnect = false;
    g_idle_after_disconnect = false;
    saved_names_remember_bond(g_current_bond_index, g_current_mouse_name);

    if (g_vendor_registered)
        g_vendor_backend.session(g_vendor_backend.context, true);

    (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_PROMOTED);
    service_vendor_output();
}

static void pair_new_begin_handoff(void)
{
    stop_pair_new_timer();
    g_pair_new_active = false;
    g_pair_new_state = BLE_PAIR_NEW_READY;

    if (g_state == BLE_HOGP_STATE_READY &&
        g_connection_handle != HCI_CON_HANDLE_INVALID) {
        g_pair_new_handoff_pending = true;
        if (g_vendor_registered)
            g_vendor_backend.session(g_vendor_backend.context, false);
        g_state = BLE_HOGP_STATE_DISCONNECTING;
        gap_disconnect(g_connection_handle);
        return;
    }

    pair_new_finalize_promotion();
}

static void service_vendor_output(void)
{
    if (!g_vendor_registered || g_state != BLE_HOGP_STATE_READY || g_hids_cid == 0u) return;
    uint8_t report_id = 0u;
    uint8_t payload[BLU2USB_BLE_HOGP_VENDOR_OUTPUT_MAX] = {0};
    uint16_t payload_len = 0u;
    if (!g_vendor_backend.next_output(g_vendor_backend.context, &report_id,
        payload, &payload_len, (uint16_t)sizeof(payload))) return;
    const bool valid = report_id != 0u && payload_len > 0u && payload_len <= sizeof(payload);
    const uint8_t status = valid ? hids_client_send_write_report(
        g_hids_cid, report_id, HID_REPORT_TYPE_OUTPUT, payload, (uint8_t)payload_len)
        : ERROR_CODE_PARAMETER_OUT_OF_MANDATORY_RANGE;
    g_vendor_backend.output_result(g_vendor_backend.context,
                                   status == ERROR_CODE_SUCCESS);
}

static void service_saved_search_requests(void)
{
    if (atomic_exchange_explicit(&g_saved_search_cancel, false, memory_order_acq_rel)) {
        if (g_saved_search_active) {
            stop_reconnect_timer();
            g_saved_search_active = false;
            if (g_state == BLE_HOGP_STATE_CONNECTING) {
                g_reconnect_cancel_pending = true;
                if (gap_connect_cancel() != ERROR_CODE_SUCCESS) {
                    g_reconnect_cancel_pending = false;
                    g_state = BLE_HOGP_STATE_IDLE;
                }
            } else if (g_connection_handle != HCI_CON_HANDLE_INVALID &&
                       g_state != BLE_HOGP_STATE_READY) {
                g_idle_after_disconnect = true;
                g_state = BLE_HOGP_STATE_DISCONNECTING;
                gap_disconnect(g_connection_handle);
            } else if (g_state != BLE_HOGP_STATE_READY) {
                g_state = BLE_HOGP_STATE_IDLE;
            }
        }
    }

    if (atomic_exchange_explicit(&g_saved_search_request, false, memory_order_acq_rel)) {
        if (g_pair_new_state != BLE_PAIR_NEW_IDLE ||
            g_pair_new_handoff_pending || g_pair_new_cancel_pending) {
            atomic_store_explicit(&g_saved_search_request, true, memory_order_release);
            return;
        }
        if (!g_saved_search_active) {
            if (g_state == BLE_HOGP_STATE_SCANNING) gap_stop_scan();
            if (g_state != BLE_HOGP_STATE_READY &&
                g_state != BLE_HOGP_STATE_DISCONNECTING) {
                if (!start_bonded_reconnect())
                    g_state = BLE_HOGP_STATE_IDLE;
            }
        }
    }
}

static void service_pair_new_cancel_request(void)
{
    if (atomic_exchange_explicit(&g_pair_new_cancel, false, memory_order_acq_rel))
        cancel_pair_new();
}

static void service_pair_new_requests(void)
{
    if (atomic_exchange_explicit(&g_pair_new_request, false, memory_order_acq_rel)) {
        if (!start_pair_new())
            atomic_store_explicit(&g_pair_new_request, true, memory_order_release);
    }
}

static void vendor_timer_handler(btstack_timer_source_t *timer)
{
    (void)timer;
    service_pair_new_cancel_request();
    service_saved_search_requests();
    service_pair_new_requests();
    service_vendor_output();
    btstack_run_loop_set_timer(&g_vendor_timer, BLE_HOGP_VENDOR_SERVICE_MS);
    btstack_run_loop_add_timer(&g_vendor_timer);
}

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
        g_current_bond_index = resolve_current_bond_index();
        saved_names_remember_bond(g_current_bond_index, g_current_mouse_name);
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

        if (g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid)
            break;

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

        if (g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid)
            break;

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
            g_state == BLE_HOGP_STATE_WAITING_FOR_STACK) {
            saved_names_load();
            reconnect_or_scan();
        }
        break;

    case GAP_EVENT_ADVERTISING_REPORT: {
        if (!advertisement_has_hid_service(packet)) break;

        bd_addr_t address;
        gap_event_advertising_report_get_address(packet, address);
        const bd_addr_type_t type =
            gap_event_advertising_report_get_address_type(packet);
        const uint16_t appearance = advertisement_appearance(packet);

        if (g_pair_new_active && g_pair_new_state == BLE_PAIR_NEW_SCANNING) {
            if (address_is_rejected(address, type)) break;
            if (appearance_is_explicit_non_mouse_hid(appearance)) {
                reject_address(address, type);
                break;
            }
            if (address_is_saved(address, type)) break;

            gap_stop_scan();
            memcpy(g_pair_new_address, address, sizeof(bd_addr_t));
            g_pair_new_address_type = type;
            g_pair_new_cancel_pending = false;
            g_pair_new_state = BLE_PAIR_NEW_CONNECTING;
            if (gap_connect(g_pair_new_address, g_pair_new_address_type) !=
                ERROR_CODE_SUCCESS) {
                pair_new_resume_scan();
            }
            break;
        }

        if (g_state != BLE_HOGP_STATE_SCANNING) break;
        if (address_is_rejected(address, type)) break;
        if (appearance_is_explicit_non_mouse_hid(appearance)) {
            reject_address(address, type);
            break;
        }

        gap_stop_scan();
        stop_reconnect_timer();
        memcpy(g_remote_address, address, sizeof(bd_addr_t));
        g_remote_address_type = type;
        g_reconnect_cancel_pending = false;
        g_state = BLE_HOGP_STATE_CONNECTING;
        if (gap_connect(g_remote_address, g_remote_address_type) !=
            ERROR_CODE_SUCCESS) {
            start_scan();
        }
        break;
    }

    case HCI_EVENT_META_GAP:
        if (hci_event_gap_meta_get_subevent_code(packet) !=
            GAP_SUBEVENT_LE_CONNECTION_COMPLETE) {
            break;
        }

        if (g_pair_new_state == BLE_PAIR_NEW_CONNECTING) {
            const uint8_t status =
                gap_subevent_le_connection_complete_get_status(packet);

            if (g_pair_new_cancel_pending) {
                g_pair_new_cancel_pending = false;
                if (status == ERROR_CODE_SUCCESS) {
                    g_pair_new_connection_handle =
                        gap_subevent_le_connection_complete_get_connection_handle(packet);
                    pair_new_disconnect_candidate(false, false);
                } else {
                    pair_new_clear_candidate();
                }
                break;
            }

            if (status != ERROR_CODE_SUCCESS) {
                g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
                if (g_pair_new_active) pair_new_resume_scan();
                else pair_new_clear_candidate();
                break;
            }

            g_pair_new_connection_handle =
                gap_subevent_le_connection_complete_get_connection_handle(packet);
            g_pair_new_state = BLE_PAIR_NEW_SECURING;
            sm_request_pairing(g_pair_new_connection_handle);
            break;
        }

        if (g_state == BLE_HOGP_STATE_CONNECTING) {
            const uint8_t status =
                gap_subevent_le_connection_complete_get_status(packet);
            if (g_reconnect_cancel_pending) {
                g_reconnect_cancel_pending = false;
                if (status == ERROR_CODE_SUCCESS) {
                    g_connection_handle =
                        gap_subevent_le_connection_complete_get_connection_handle(packet);
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
                    (void)publish_status(
                        BLU2USB_BLE_HOGP_MESSAGE_SAVED_SEARCH_TIMEOUT);
                    g_state = BLE_HOGP_STATE_IDLE;
                } else {
                    start_scan();
                }
                break;
            }

            if (!g_saved_search_active) stop_reconnect_timer();
            g_remote_address_type = (bd_addr_type_t)
                gap_subevent_le_connection_complete_get_peer_address_type(packet);
            gap_subevent_le_connection_complete_get_peer_address(
                packet, g_remote_address);
            g_connection_handle =
                gap_subevent_le_connection_complete_get_connection_handle(packet);
            g_state = BLE_HOGP_STATE_SECURING;
            sm_request_pairing(g_connection_handle);
        }
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE: {
        const hci_con_handle_t disconnected =
            hci_event_disconnection_complete_get_connection_handle(packet);

        if (g_pair_new_handoff_pending &&
            disconnected == g_connection_handle) {
            g_connection_handle = HCI_CON_HANDLE_INVALID;
            g_hids_cid = 0u;
            memset(&g_parser, 0, sizeof(g_parser));
            g_current_mouse_name[0] = '\0';
            pair_new_finalize_promotion();
            break;
        }

        if (disconnected == g_pair_new_connection_handle) {
            const bool resume =
                g_pair_new_resume_after_disconnect &&
                g_pair_new_active && g_pair_new_timer_active;
            g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
            g_pair_new_hids_cid = 0u;
            memset(&g_pair_new_parser, 0, sizeof(g_pair_new_parser));
            g_pair_new_resume_after_disconnect = false;
            g_pair_new_cancel_pending = false;
            g_pair_new_state = BLE_PAIR_NEW_IDLE;
            if (resume) pair_new_resume_scan();
            break;
        }

        if (g_idle_after_disconnect) {
            g_idle_after_disconnect = false;
            g_connection_handle = HCI_CON_HANDLE_INVALID;
            g_hids_cid = 0u;
            memset(&g_parser, 0, sizeof(g_parser));
            g_current_mouse_name[0] = '\0';
            g_state = BLE_HOGP_STATE_IDLE;
            break;
        }

        const bool was_ready = g_state == BLE_HOGP_STATE_READY;
        const bool reconnect_bonded = was_ready || g_reconnect_after_disconnect;
        stop_reconnect_timer();

        if (was_ready && g_vendor_registered)
            g_vendor_backend.session(g_vendor_backend.context, false);

        g_connection_handle = HCI_CON_HANDLE_INVALID;
        g_hids_cid = 0u;
        memset(&g_parser, 0, sizeof(g_parser));
        g_current_mouse_name[0] = '\0';

        if (was_ready)
            (void)publish_status(BLU2USB_BLE_HOGP_MESSAGE_DISCONNECTED);

        g_reconnect_after_disconnect = false;

        if (g_pair_new_active || g_pair_new_handoff_pending) {
            g_state = BLE_HOGP_STATE_IDLE;
            break;
        }

        if (reconnect_bonded) reconnect_or_scan();
        else start_scan();
        break;
    }

    default:
        break;
    }
}

static void sm_packet_handler(uint8_t packet_type, uint16_t channel,
                              uint8_t *packet, uint16_t size)
{
    (void)channel; (void)size;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
    case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED: {
        const hci_con_handle_t handle =
            sm_event_identity_created_get_handle(packet);
        const int index =
            sm_event_identity_resolving_succeeded_get_index(packet);
        if (bond_slot_info(index, NULL, NULL, NULL)) {
            if (handle == g_connection_handle)
                g_current_bond_index = index;
            if (handle == g_pair_new_connection_handle)
                g_pair_new_bond_index = index;
        }
        break;
    }

    case SM_EVENT_IDENTITY_CREATED: {
        const hci_con_handle_t handle =
            sm_event_identity_created_get_handle(packet);
        const int index = sm_event_identity_created_get_index(packet);
        if (bond_slot_info(index, NULL, NULL, NULL)) {
            if (handle == g_connection_handle)
                g_current_bond_index = index;
            if (handle == g_pair_new_connection_handle)
                g_pair_new_bond_index = index;
        }
        break;
    }

    case SM_EVENT_JUST_WORKS_REQUEST:
        sm_just_works_confirm(
            sm_event_just_works_request_get_handle(packet));
        break;

    case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
        sm_numeric_comparison_confirm(
            sm_event_passkey_display_number_get_handle(packet));
        break;

    case SM_EVENT_PAIRING_COMPLETE: {
        const hci_con_handle_t handle =
            sm_event_pairing_complete_get_handle(packet);
        const bool success =
            sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS;

        if (handle == g_pair_new_connection_handle &&
            g_pair_new_state == BLE_PAIR_NEW_SECURING) {
            if (success) read_pair_new_mouse_name();
            else pair_new_disconnect_candidate(true, true);
            break;
        }

        if (handle == g_connection_handle &&
            g_state == BLE_HOGP_STATE_SECURING) {
            if (success) read_current_mouse_name();
            else disconnect_and_rescan();
        }
        break;
    }

    case SM_EVENT_REENCRYPTION_COMPLETE: {
        const hci_con_handle_t handle =
            sm_event_reencryption_complete_get_handle(packet);
        const bool success =
            sm_event_reencryption_complete_get_status(packet) ==
                ERROR_CODE_SUCCESS;

        if (handle == g_pair_new_connection_handle &&
            g_pair_new_state == BLE_PAIR_NEW_SECURING) {
            pair_new_disconnect_candidate(false,
                success && g_pair_new_active);
            break;
        }

        if (handle == g_connection_handle &&
            g_state == BLE_HOGP_STATE_SECURING) {
            if (success) read_current_mouse_name();
            else disconnect_and_rescan();
        }
        break;
    }

    default:
        break;
    }
}

static void ble_hogp_session_setup(void)
{
    memset(&g_parser, 0, sizeof(g_parser));
    memset(g_rejected_devices, 0, sizeof(g_rejected_devices));
    memset(g_saved_names, 0, sizeof(g_saved_names));
    g_saved_names_loaded = false;
    memset(g_remote_address, 0, sizeof(g_remote_address));
    g_remote_address_type = BD_ADDR_TYPE_UNKNOWN;
    g_rejected_next = 0u;
    g_state = BLE_HOGP_STATE_WAITING_FOR_STACK;
    g_connection_handle = HCI_CON_HANDLE_INVALID;
    g_current_bond_index = -1;
    g_hids_cid = 0u;
    g_current_mouse_name[0] = '\0';
    g_reconnect_timer_active = false;
    g_reconnect_cancel_pending = false;
    g_reconnect_after_disconnect = false;
    g_idle_after_disconnect = false;
    g_saved_search_active = false;
    atomic_store_explicit(&g_saved_search_request, false, memory_order_relaxed);
    atomic_store_explicit(&g_saved_search_cancel, false, memory_order_relaxed);

    g_pair_new_state = BLE_PAIR_NEW_IDLE;
    g_pair_new_active = false;
    g_pair_new_cancel_pending = false;
    g_pair_new_resume_after_disconnect = false;
    g_pair_new_handoff_pending = false;
    g_pair_new_bond_count_before = 0;
    g_pair_new_bond_index = -1;
    memset(g_pair_new_address, 0, sizeof(g_pair_new_address));
    g_pair_new_address_type = BD_ADDR_TYPE_UNKNOWN;
    g_pair_new_connection_handle = HCI_CON_HANDLE_INVALID;
    g_pair_new_hids_cid = 0u;
    memset(&g_pair_new_parser, 0, sizeof(g_pair_new_parser));
    g_pair_new_timer_active = false;
    atomic_store_explicit(&g_pair_new_request, false, memory_order_relaxed);
    atomic_store_explicit(&g_pair_new_cancel, false, memory_order_relaxed);

    hids_client_init(g_descriptor_storage, sizeof(g_descriptor_storage));
    g_hci_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&g_hci_registration);
    g_sm_registration.callback = &sm_packet_handler;
    sm_add_event_handler(&g_sm_registration);
    btstack_run_loop_set_timer_handler(&g_reconnect_timer, reconnect_timeout_handler);
    btstack_run_loop_set_timer_handler(&g_pair_new_timer, pair_new_timeout_handler);
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

int blu2usb_ble_hogp_pico_current_bond_index(void)
{
    return bond_ordinal_for_slot(resolve_current_bond_index());
}

bool blu2usb_ble_hogp_pico_saved_mouse_name(int bond_index,
                                            char *out,
                                            size_t out_capacity)
{
    if (out == NULL || out_capacity == 0u) return false;
    out[0] = '\0';
    saved_names_load();

    const int db_slot = bond_slot_for_ordinal(bond_index);
    bd_addr_type_t address_type = BD_ADDR_TYPE_UNKNOWN;
    bd_addr_t address;
    if (!saved_identity_for_bond(db_slot, &address_type, address)) return false;

    const int slot = saved_names_find(address_type, address);
    if (slot < 0 || g_saved_names[slot].name[0] == '\0') return false;

    size_t length = 0u;
    while (g_saved_names[slot].name[length] != '\0' &&
           length + 1u < out_capacity) {
        out[length] = g_saved_names[slot].name[length];
        ++length;
    }
    out[length] = '\0';
    return length > 0u;
}

const char *blu2usb_ble_hogp_pico_current_mouse_name(void)
{
    return g_current_mouse_name;
}

void blu2usb_ble_hogp_pico_request_saved_search(void)
{
    atomic_store_explicit(&g_saved_search_request, true, memory_order_release);
}

void blu2usb_ble_hogp_pico_cancel_saved_search(void)
{
    atomic_store_explicit(&g_saved_search_cancel, true, memory_order_release);
}

void blu2usb_ble_hogp_pico_request_pair_new(void)
{
    atomic_store_explicit(&g_pair_new_request, true, memory_order_release);
}

void blu2usb_ble_hogp_pico_cancel_pair_new(void)
{
    atomic_store_explicit(&g_pair_new_cancel, true, memory_order_release);
}
