"""MOV-07 source contract for Pico-only BLE transitions and USB delivery."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")

def section(text, start, end):
    assert start in text, f"missing section {start!r}"
    value = text.split(start, 1)[1]
    assert end in value, f"missing ending section {end!r}"
    return value.split(end, 1)[0]

connected = section(app, "case BLU2USB_BLE_HOGP_EVENT_CONNECTED:",
                    "case BLU2USB_BLE_HOGP_EVENT_DISCONNECTED:")
assert "blu2usb_ux_set_mouse_connected(true)" in connected
assert "synchronize_saved_mice(ux, true)" in connected
assert "ux->screen == BLU2USB_SCREEN_HOME_SEARCHING" in connected
assert "ux->screen = BLU2USB_SCREEN_HOME;" in connected
assert "ux->selection = 0u;" in connected

disconnected = section(app, "case BLU2USB_BLE_HOGP_EVENT_DISCONNECTED:",
                       "case BLU2USB_BLE_HOGP_EVENT_SAVED_SEARCH_STARTED:")
assert "blu2usb_movement_pipeline_clear(motion)" in disconnected
assert "blu2usb_ux_set_mouse_connected(false)" in disconnected
assert "synchronize_saved_mice(ux, false)" in disconnected
assert "ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS" in disconnected
assert "ux->screen <= BLU2USB_SCREEN_HELP_ROTATION" in disconnected
assert "ux->screen == BLU2USB_SCREEN_HOME" in disconnected
assert disconnected.count("ux->screen = BLU2USB_SCREEN_HOME_SEARCHING;") >= 2
assert "ux->return_screen = BLU2USB_SCREEN_HOME_SEARCHING;" in disconnected

timed_out = section(app, "case BLU2USB_BLE_HOGP_EVENT_SAVED_SEARCH_TIMEOUT:",
                    "case BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_STARTED:")
assert "ux->screen == BLU2USB_SCREEN_HOME_SEARCHING" in timed_out
assert "ux->screen = BLU2USB_SCREEN_HOME_RETRY;" in timed_out

saved_sync = section(app, "static void synchronize_saved_mice(", "static bool service_ble_messages(")
assert "blu2usb_ux_set_movement(" in saved_sync
assert "blu2usb_ble_hogp_pico_saved_movement(bond)" in saved_sync
assert "blu2usb_ux_set_saved_connected_bond(ux, bond)" in saved_sync

# Movement output is serviced on every loop, even when LCD is locked.
loop = section(app, "for (;;) {", "blu2usb_hat_pico_task();")
assert "service_usb_mouse(&aggregator, &motion, ux.movement_current," in loop
assert "service_usb_keyboard(&aggregator, &last_keyboard, &last_keyboard_valid)" in loop
assert "if (ble_ui_changed && !blu2usb_interaction_is_locked(&ux.interaction))" in loop

assert 'BLU2USB_SCREEN_HOME] = {{"UNKNOWN MOUSE"," NO REMAP PASSTHROUGH"," MOVEMENT OPTIONS"," SAVED DEVICES"," PAIR NEW MOUSE"' in ux
assert "case BLU2USB_SCREEN_HOME: return 4;" in ux
assert "case BLU2USB_SCREEN_HOME_SEARCHING:" in ux
assert "case BLU2USB_SCREEN_HOME_RETRY: return 2;" in ux
assert "ux->return_selection = ux->selection;" in ux

print("MOV-07 Pico BLE, HOME, lock-independent HID invariants: PASS")
