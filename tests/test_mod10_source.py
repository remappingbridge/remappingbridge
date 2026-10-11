"""MOD-10 security/ABI invariant review for the immutable candidate source.

Not a surrogate for physical USB protocol, BT reliability, or downgrade rehearsal.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
read = lambda p: (root / p).read_text(encoding="utf-8")
app = read("src/app/main.c")
usb = read("src/usb_hid/usb_descriptors.c")
usb_pico = read("src/usb_hid/usb_hid_pico.c")
usb_header = read("include/blu2usb/usb_hid/usb_hid.h")
config = read("include/tusb_config.h")
profile = read("src/profiles/profiles.c")
ble = read("src/ble_hogp/ble_hogp_pico.c")
renderer = read("src/renderer/renderer.c")
cmake = read("CMakeLists.txt")
ci = read(".github/workflows/ci.yml")

assert "BLU2USB_USB_HID_INTERFACE_COUNT 2u" in usb_header
assert "BLU2USB_USB_REPORT_ID_KEYBOARD" in usb and "BLU2USB_USB_REPORT_ID_CONSUMER" in usb
assert "TUD_HID_REPORT_DESC_CONSUMER" in usb
assert "TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL" in usb
assert "HID_PROTOCOL_BOOT" in usb_pico
assert "BLU2USB_USB_REPORT_ID_CONSUMER" in usb_pico
assert "tud_hid_n_ready" in usb_pico
assert not re.search(r"\bTUD_CDC_DESCRIPTOR\s*\(", usb)
assert not re.search(r"\bCFG_TUD_CDC\s+[1-9]", config)
assert "pico_enable_stdio_usb(blu2usb_picow 0)" in cmake or "PICO_STDIO_USB" in cmake
assert "pico2_w" in cmake
assert "blu2usb_picow.uf2" in ci
assert "blu2usb_picow_debug.uf2" in ci  # explicitly prohibited by test ! -e
assert "test ! -e build-pico/blu2usb_picow_debug.uf2" in ci

assert "BLU2USB_PROFILE_V3_SERIALIZED_SIZE" in profile
assert "blu2usb_profiles_restore_any" in profile
assert "candidate.shortcuts" in profile
assert "g_profile_writes_allowed = false" in app
assert "persist_profiles(profiles)" in app
assert "blu2usb_ux_shortcut_committed" in app
assert "blu2usb_modifier_shortcuts_cancel(shortcuts, aggregator)" in app
assert "BLU2USB_BLE_HOGP_EVENT_DISCONNECTED" in app
assert "BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED" in app
assert "blu2usb_bt_runtime_take_overflow" in app
assert "discard_stale_mouse_motion" in app
assert "last_consumer_sent != 0u" in app
assert "blu2usb_usb_hid_pico_send_consumer(0u)" in app
assert "movements_remove_identity" in ble  # identity-scoped removal
assert "gap_load_resolving_list_from_le_device_db" in ble
assert "row == 1u && connected_name" in renderer
assert "BLU2USB_UI_TONE_STATIC" in renderer
print("MOD-10 release security and ABI invariants: PASS")
