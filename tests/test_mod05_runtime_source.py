from pathlib import Path
r=Path(__file__).resolve().parents[1]
main=(r/"src/app/main.c").read_text()
hid=(r/"include/blu2usb/domain/hid.h").read_text()
core=(r/"src/modifier_shortcuts/modifier_shortcuts.c").read_text()
for name in ("blu2usb_modifier_shortcuts_configure", "blu2usb_modifier_shortcuts_process",
             "blu2usb_modifier_shortcuts_prepare_usb", "blu2usb_modifier_shortcuts_ack_usb",
             "blu2usb_modifier_shortcuts_cancel", "blu2usb_modifier_shortcuts_assignments"):
    assert name in main, name
assert "BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT" in hid
assert "blu2usb_keyboard_shortcut_lookup" in core
assert "keyboard_key_available" in core
assert "BLU2USB_KEY_ESCAPE" in core
assert "queue_action(state, state->assignments[0])" in core
assert "const bool keyboard_accepted" in main
assert "blu2usb_modifier_shortcuts_ack_usb(&shortcuts, keyboard_accepted)" in main
assert "blu2usb_modifier_shortcuts_process_event(" in main
assert main.index("blu2usb_modifier_shortcuts_process_event(")<main.index("blu2usb_remap_process_mouse(remap, &forwarded, &mapped)")
assert 'if (!blu2usb_usb_hid_pico_send_keyboard(&report)) return false;' in main
assert "blu2usb_modifier_shortcuts_cancel(&shortcuts, &aggregator)" in main
assert "BLU2USB_USB_HID_INTERFACE_COUNT 2u" in (r/"include/blu2usb/usb_hid/usb_hid.h").read_text()
print("MOD-05 physical-before-remap, source-scoped two-phase HID ACK runtime PASS")
