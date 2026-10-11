"""MOV-09: Pico-only BLE removal / per-bond preference deletion safety contract."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
ble = (root / "src/ble_hogp/ble_hogp_pico.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")


def between(content, start, end):
    assert start in content, f"missing start: {start}"
    section = content.split(start, 1)[1]
    assert end in section, f"missing end: {end}"
    return section.split(end, 1)[0]


request = between(ble, "static void service_remove_saved_request(void)\n{",
                  "static void service_pair_new_cancel_request(void)")
assert "atomic_exchange_explicit(" in request
assert "&g_remove_saved_request, -1" in request
assert "capture_logical_identity(logical_bond, &address_type, address, irk)" in request
# Stale logical bonds must never strand UI in pending REMOVE THIS.
stale = between(request,
                "if (!capture_logical_identity(logical_bond, &address_type, address, irk)) {",
                "g_remove_target_valid = true;")
assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVE_FAILED" in stale
assert "return;" in stale
assert "current_matches_identity(address_type, address, irk)" in request
assert "gap_disconnect(g_connection_handle)" in request
assert "complete_remove_target();" in request

complete = between(ble, "static void complete_remove_target(void)\n{",
                   "static void dedupe_current_bond(void)")
assert "movements_remove_identity(" in complete
assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVE_FAILED" in complete
assert "saved_names_remove_identity(" in complete
assert "remove_bonds_for_identity(" in complete
assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVED" in complete
assert complete.index("movements_remove_identity(") < complete.index("saved_names_remove_identity(")
assert complete.index("saved_names_remove_identity(") < complete.index("remove_bonds_for_identity(")
assert complete.index("remove_bonds_for_identity(") < complete.index("BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVED")

preferences = between(ble, "static bool movements_remove_identity(", "static void complete_remove_target(")
assert "memset(&g_movements[idx], 0, sizeof(g_movements[idx]))" in preferences
assert "if (movements_store()) return true;" in preferences
assert "g_movements[idx] = prior;" in preferences
assert "return false;" in preferences

dispatch = between(app, "case BLU2USB_UX_COMMAND_REMOVE_MOUSE:", "default:")
assert "command.saved_bond >= 0" in dispatch
assert "ux->saved_connected_bond == command.saved_bond" in dispatch
assert "blu2usb_hid_aggregator_release_source(" in dispatch
assert "blu2usb_ble_hogp_pico_request_remove_saved_mouse(" in dispatch

failure = between(app, "case BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVE_FAILED:",
                  "case BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED:")
assert "ux->remove_pending = false;" in failure
assert "ux->screen = BLU2USB_SCREEN_SAVED_DEVICES;" in failure
assert "synchronize_saved_mice(" in failure

success = between(app, "case BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED:",
                  "case BLU2USB_BLE_HOGP_EVENT_MOUSE:")
for needle in ["ux->remove_pending = false;", "ux->remove_target_bond = -1;",
               "ux->saved_front_bond = -1;", "synchronize_saved_mice(",
               "BLU2USB_SCREEN_SEARCHING_FIRST", "BLU2USB_SCREEN_SAVED_DEVICES"]:
    assert needle in success, f"removed event missing {needle}"

command = between(ux, "if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&\n        control == BLU2USB_CONTROL_KEY_A) {",
                  "if (ux->screen == BLU2USB_SCREEN_APPLY_PASSTHROUGH")
assert "if (ux->remove_pending) return cmd;" in command
assert "cmd.kind = BLU2USB_UX_COMMAND_REMOVE_MOUSE" in command
assert "cmd.saved_bond = ux->remove_target_bond" in command

print("MOV-09 scoped BLE remove + movement TLV rollback + stale-bond failure: PASS")
