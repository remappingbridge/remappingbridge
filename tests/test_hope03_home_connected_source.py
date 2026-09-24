from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ble = (root / "src/ble_hogp/ble_hogp_pico.c").read_text(encoding="utf-8")
ble_h = (root / "include/blu2usb/ble_hogp/ble_hogp.h").read_text(encoding="utf-8")

# HOME is replaced in-place; no parallel HOME_CONNECTED enum is introduced.
assert "BLU2USB_SCREEN_HOME_CONNECTED" not in ux_h
assert '[BLU2USB_SCREEN_HOME] = {{"UNKNOWN MOUSE"," NO REMAP PASSTHROUGH"," SAVED DEVICES"," PAIR NEW MOUSE"," LEARN THE KEYS"' in ux
assert '{"HOME"," STATUS"," MOUSE OPTIONS"," OTHER OPTIONS"' not in ux

# Dynamic identity/profile presentation.
assert "current_mouse_name[BLU2USB_UX_MOUSE_NAME_CAPACITY]" in ux_h
assert "blu2usb_ux_set_current_mouse_name" in ux
assert "home_title(ux->current_mouse_name" in renderer
assert '" REMAPPED TO STANDARD"' in renderer
assert '" REMAPPED TO ESCAPE"' in renderer
assert '" REMAPPED TO CUSTOM"' in renderer
assert '" NO REMAP PASSTHROUGH"' in renderer

# HOME navigation is the canonical four-row order.
home_dest = """static const blu2usb_screen_id_t dest[4] = {
            BLU2USB_SCREEN_MOUSE_OPTIONS,
            BLU2USB_SCREEN_SAVED_DEVICES,
            BLU2USB_SCREEN_PAIR_MOUSE,
            BLU2USB_SCREEN_LEARN_KEYS
        };"""
assert home_dest in ux

# HOPE-28 now canonically owns the contextual HOME Help. HOPE-03 continues
# freezing the HOME shell and must not forbid the next accepted screen.
assert "BLU2USB_SCREEN_HELP_HOME_CONNECTED" in ux

# Device Name is read after security and before HIDS, best effort.
assert "ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME" in ble
assert "BLE_HOGP_STATE_READING_NAME" in ble
assert "BLE_PAIR_NEW_READING_NAME" in ble
assert "read_current_mouse_name();" in ble
assert "read_pair_new_mouse_name();" in ble
assert "if (status != ERROR_CODE_SUCCESS)\n        connect_hid_service();" in ble
assert "if (status != ERROR_CODE_SUCCESS)\n        pair_new_connect_hid_service();" in ble
assert "gatt_event_characteristic_value_query_result_get_value" in ble

# Candidate identity stays separate until atomic promotion.
assert "g_pair_new_mouse_name[BLE_HOGP_MOUSE_NAME_CAPACITY]" in ble
assert "memcpy(g_current_mouse_name, g_pair_new_mouse_name" in ble

# App synchronizes the authoritative name on normal connect and Pair New promotion
# through the centralized saved-Mouse synchronization path, and clears only the
# connection-scoped HOME name on disconnect.
assert "static void synchronize_saved_mice" in app
assert "const char *current_name = blu2usb_ble_hogp_pico_current_mouse_name();" in app
assert app.count("synchronize_saved_mice(ux, true)") >= 2
assert "blu2usb_ux_set_current_mouse_name(ux, NULL);" in app
assert "const char *blu2usb_ble_hogp_pico_current_mouse_name(void);" in ble_h

# Disconnect while HOME is visible resolves to home-searching. G06 BLE reconnect
# then publishes SAVED_SEARCH_STARTED for its accepted 8-second saved search.
assert "ux->screen == BLU2USB_SCREEN_HOME &&" in app
assert "ux->screen = BLU2USB_SCREEN_HOME_SEARCHING;" in app
assert "BLU2USB_BLE_HOGP_MESSAGE_SAVED_SEARCH_STARTED" in ble

# Bundled accepted post-HOPE-27 improvement.
assert '"MOUSE NOT FOUND HELP"' in ux
assert '"DEVICE NOT FOUND HELP"' not in ux

print("HOPE-03 home-connected source invariants: OK")
