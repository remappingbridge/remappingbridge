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
assert "count == 0 ? 1u : count" in ux
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

for legacy_text in ("PAIR KEYBOARD", "PAIR COMPOSITE", "OTHER OPTIONS"):
    assert legacy_text not in ux

assert "project_saved_devices" in renderer
assert '"STATUS: CONNECTED"' in renderer
assert '"STATUS: DISCONNECTED"' in renderer
assert '"STANDARD"' in renderer
assert "saved_current_page" in renderer
assert "blu2usb_ble_hogp_pico_current_bond_index" in ble_h
assert "blu2usb_ble_hogp_pico_current_bond_index" in ble
assert "static int g_current_bond_index = -1;" in ble
assert "SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED" in ble
assert "sm_event_identity_resolving_succeeded_get_index(packet)" in ble
assert "g_current_bond_index = bonded_count - 1;" in ble
assert "sm_le_device_index(g_connection_handle)" in ble
assert "return count == 1 ? 0 : -1;" in ble
assert "blu2usb_ux_set_saved_current_page" in app

print("HOPE-04 saved-devices source invariants: OK")
