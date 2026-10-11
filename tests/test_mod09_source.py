"""MOD-09 production-path contract assertions, beyond host-domain behavior."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
app = (ROOT / "src/app/main.c").read_text(encoding="utf-8")
renderer = (ROOT / "src/renderer/renderer.c").read_text(encoding="utf-8")
mod = (ROOT / "src/modifier_shortcuts/modifier_shortcuts.c").read_text(encoding="utf-8")

assert "discard_stale_mouse_motion(&aggregator, &motion)" in app
assert "discard_stale_mouse_motion(aggregator, motion)" in app
assert "blu2usb_bt_runtime_take_overflow()" in app
assert "blu2usb_modifier_shortcuts_cancel(shortcuts, aggregator);\n            blu2usb_modifier_shortcuts_assignments" in app
assert "blu2usb_modifier_shortcuts_cancel(&shortcuts, &aggregator);" in app
assert "last_consumer_sent != 0u" in app and "blu2usb_usb_hid_pico_send_consumer(0u)" in app
assert "BLU2USB_BLE_HOGP_EVENT_SAVED_MOUSE_REMOVED" in app
assert "synchronize_saved_mice(" in app
assert "BLU2USB_SCREEN_SAVED_DEVICES" in renderer
assert "row == 1u && connected_name" in renderer
assert "BLU2USB_UI_TONE_STATIC" in renderer
assert "blu2usb_modifier_shortcuts_action_supported(state->assignments[combo])" in mod
print("MOD-09 app / renderer / cancel guards: PASS")
