from pathlib import Path

root = Path(__file__).resolve().parents[1]
ble = (root / "src/ble_hogp/ble_hogp_pico.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ble_h = (root / "include/blu2usb/ble_hogp/ble_hogp.h").read_text(encoding="utf-8")
btstack_config = (root / "include/btstack_config.h").read_text(encoding="utf-8")

# Pair New is a distinct bounded 15-second operation.
assert "#define BLE_HOGP_PAIR_NEW_TIMEOUT_MS 15000u" in ble
assert "btstack_run_loop_set_timer(&g_pair_new_timer, BLE_HOGP_PAIR_NEW_TIMEOUT_MS)" in ble

# The static BTstack pools must actually permit the authoritative session plus
# one temporary Pair New candidate. This caught the first physical HOPE-06 failure:
# the implementation had two logical sessions but BTstack was capped at one.
assert "#define MAX_NR_HCI_CONNECTIONS 2" in btstack_config
assert "#define MAX_NR_GATT_CLIENTS 2" in btstack_config
assert "#define MAX_NR_HIDS_CLIENTS 2" in btstack_config

# Candidate state is separate from the authoritative G06 session.
for symbol in (
    "g_pair_new_connection_handle",
    "g_pair_new_hids_cid",
    "g_pair_new_parser",
    "g_pair_new_address",
):
    assert symbol in ble

# Existing bonds are excluded both before connection and after security.
assert "address_is_saved(address, type)" in ble
assert "SM_EVENT_REENCRYPTION_COMPLETE" in ble
assert "pair_new_created_new_bond()" in ble

# Multi-HIDS routing is CID-specific. Candidate reports never flow to product input.
assert "gattservice_subevent_hid_service_connected_get_hids_cid" in ble
assert "gattservice_subevent_hid_report_get_hids_cid" in ble
assert "gattservice_subevent_hid_service_disconnected_get_hids_cid" in ble
candidate_report_guard = """if (g_pair_new_hids_cid != 0u && cid == g_pair_new_hids_cid)
            break;"""
assert candidate_report_guard in ble

# Promotion happens only after candidate Mouse parser qualification.
assert "blu2usb_ble_hogp_parser_has_mouse(&g_pair_new_parser)" in ble
assert "pair_new_begin_handoff();" in ble
assert "pair_new_finalize_promotion();" in ble
assert "BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_PROMOTED" in ble
assert "g_connection_handle = g_pair_new_connection_handle;" in ble
assert "g_parser = g_pair_new_parser;" in ble

# Current live session stays authoritative until handoff; old product HID state is
# explicitly released when promotion reaches the app.
promoted = app.index("case BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_PROMOTED:")
release_mouse = app.index("blu2usb_hid_aggregator_release_source(aggregator, mouse)", promoted)
release_synth = app.index("blu2usb_hid_aggregator_release_source(aggregator, synthetic)", promoted)
assert promoted < release_mouse < release_synth

# UI entry/exit controls the candidate lifecycle.
assert "blu2usb_ble_hogp_pico_request_pair_new();" in app
assert "blu2usb_ble_hogp_pico_cancel_pair_new();" in app
assert "BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_TIMEOUT" in app

# The legacy Pair Mouse visual/help was removed in-place.
assert '"PAIR NEW MOUSE"' in ux
assert '"SEARCHING BLE HID","TARGET MOUSE"' not in ux
assert "BLU2USB_SCREEN_PAIR_MOUSE_HELP" not in ux_h
assert "BLU2USB_SCREEN_PAIR_MOUSE_HELP" not in ux

# No future Pair New Help is introduced by this gate.
assert "PAIR NEW DEVICE HELP" not in ux

# Runtime event contract is wired end-to-end.
for token in (
    "BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_STARTED",
    "BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_TIMEOUT",
    "BLU2USB_BLE_HOGP_MESSAGE_PAIR_NEW_PROMOTED",
    "BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_STARTED",
    "BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_TIMEOUT",
    "BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_PROMOTED",
):
    assert token in ble_h

print("HOPE-06 Pair New source invariants: OK")
