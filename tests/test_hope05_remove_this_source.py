from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
ble_h = (root / "include/blu2usb/ble_hogp/ble_hogp.h").read_text(encoding="utf-8")
ble = (root / "src/ble_hogp/ble_hogp.c").read_text(encoding="utf-8")
pico = (root / "src/ble_hogp/ble_hogp_pico.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_REMOVE_THIS" in ux_h
assert "BLU2USB_UX_COMMAND_REMOVE_MOUSE" in ux_h
assert "bool remove_pending;" in ux_h
assert "int saved_bond;" in ux_h
assert '"REMOVE THIS MOUSE","UNKNOWN MOUSE",EMPTY,"PAIRING AND MAPPINGS","WILL BE DELETED",EMPTY,"KEY A: REMOVE","KEY B: CANCEL","KEY X: HELP"' in ux
assert "case BLU2USB_SCREEN_REMOVE_THIS: return BLU2USB_SCREEN_SAVED_DEVICES;" in ux
assert "ux->remove_pending = true;" in ux
assert "cmd.saved_bond = bond;" in ux
assert "HOPE-30 owns KEY X / help-remove-this" in ux
assert "BLU2USB_SCREEN_HELP_REMOVE_THIS" not in ux_h

assert "project_remove_this" in renderer
assert "home_title(saved_name, name);" in renderer
assert "current ? BLU2USB_UI_TONE_CURRENT : BLU2USB_UI_TONE_STATIC" in renderer

assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVED" in ble_h
assert "BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED" in ble_h
assert "blu2usb_ble_hogp_pico_request_remove_saved_mouse" in ble_h
assert "BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED" in ble

assert "atomic_int g_remove_saved_request" in pico
assert "service_remove_saved_request" in pico
assert "capture_logical_identity" in pico
assert "saved_names_remove_identity" in pico
assert "remove_bonds_for_identity" in pico
assert "le_device_db_remove(slot)" in pico
assert "bond_identity_equal" in pico
assert "g_remove_disconnect_pending" in pico
assert "BLU2USB_BLE_HOGP_MESSAGE_DISCONNECTED" in pico
assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVED" in pico
assert "bond_unique_count() > 0" in pico
assert "start_scan();" in pico

assert "case BLU2USB_UX_COMMAND_REMOVE_MOUSE:" in app
assert "blu2usb_ble_hogp_pico_request_remove_saved_mouse" in app
assert "blu2usb_hid_aggregator_release_source" in app
assert "case BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED:" in app
assert "ux->remove_pending = false;" in app
assert "BLU2USB_SCREEN_LEARN_KEYS" in app
assert "BLU2USB_SCREEN_SAVED_DEVICES" in app

print("HOPE-05 remove-this source invariants: OK")
