from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
ble_h = (root / "include/blu2usb/ble_hogp/ble_hogp.h").read_text(encoding="utf-8")
ble = (root / "src/ble_hogp/ble_hogp_pico.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")

assert '"0 OF 0","UNKNOWN MOUSE","STATUS: DISCONNECTED","PROFILE: PASSTHROUGH"," REMOVE DEVICE"' in ux
assert '"JOY RIGHT\\\\LEFT: PAGE","JOY PRESS: ACCESS","KEY B: BACK"' in ux
assert "case BLU2USB_SCREEN_SAVED_DEVICES: return BLU2USB_SCREEN_HOME;" in ux
assert "case BLU2USB_SCREEN_SAVED_DEVICES: return 0;" in ux
assert "HOPE-05 owns remove-this" in ux

for legacy in (
    "BLU2USB_SCREEN_OTHER_OPTIONS",
    "BLU2USB_SCREEN_PAIR_KEYBOARD",
    "BLU2USB_SCREEN_KEYBOARD_SAVED",
    "BLU2USB_SCREEN_PAIR_COMPOSITE",
    "BLU2USB_SCREEN_COMPOSITE_SAVED",
    "BLU2USB_SCREEN_DEVICE_DETAILS_MOUSE",
    "BLU2USB_SCREEN_DEVICE_DETAILS_KEYBOARD",
    "BLU2USB_SCREEN_DEVICE_DETAILS_COMPOSITE",
    "BLU2USB_SCREEN_REMOVE_DEVICE",
):
    assert legacy not in ux_h
    assert legacy not in ux

assert "BLU2USB_UX_MAX_SAVED_MICE 8u" in ux_h
assert "saved_connected_bond" in ux_h
assert "saved_front_bond" in ux_h
assert "saved_mouse_names" in ux_h
assert "blu2usb_ux_saved_bond_for_page" in ux
assert "ux->saved_front_bond = bond_index;" in ux
assert "ux->saved_page = 0u;" in ux
assert "ux->saved_connected_bond = -1;" in ux

assert "project_saved_devices" in renderer
assert "blu2usb_ux_saved_bond_for_page" in renderer
assert "ux->saved_mouse_names[bond]" in renderer
assert '"STATUS: CONNECTED"' in renderer
assert '"STATUS: DISCONNECTED"' in renderer

assert "blu2usb_ble_hogp_pico_saved_mouse_name" in ble_h
assert "BLE_HOGP_SAVED_REGISTRY_TAG" in ble
assert "btstack_tlv_get_instance" in ble
assert "saved_names_load" in ble
assert "saved_names_store" in ble
assert "saved_names_remember_bond" in ble
assert "saved_identity_for_bond" in ble
assert "SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED" in ble
assert "sm_event_identity_resolving_succeeded_get_index(packet)" in ble
assert "saved_names_remember_bond(g_current_bond_index, g_current_mouse_name)" in ble

assert "synchronize_saved_mice" in app
assert "blu2usb_ble_hogp_pico_saved_mouse_name" in app
assert "blu2usb_ux_set_saved_connected_bond(ux, bond)" in app
assert "blu2usb_ux_set_saved_connected_bond(ux, -1)" in app
assert "saved-device names are" in app
assert "persistent product data" in app

print("HOPE-04 saved-devices source invariants: OK")
