from pathlib import Path
root=Path(__file__).resolve().parents[1]
core=(root/"src/modifier_shortcuts/modifier_shortcuts.c").read_text()
catalog=(root/"src/modifier_shortcuts/keyboard_catalog.c").read_text()
header=(root/"include/blu2usb/modifier_shortcuts/keyboard_catalog.h").read_text()
main=(root/"src/app/main.c").read_text()
descriptor=(root/"include/blu2usb/usb_hid/usb_hid.h").read_text()
assert 'bool blu2usb_keyboard_shortcut_lookup(' in catalog
assert 'BLU2USB_KEYBOARD_SHORTCUT_MAX_ID 30u' in header
assert 'blu2usb_keyboard_shortcut_lookup(id, &spec)' in core
assert 'keyboard_key_available(aggregator, spec.key_usage)' in core
assert 'apply_modifier_mask(aggregator, spec.modifier_mask)' in core
assert 'apply_key(aggregator, spec.key_usage, true)' in core
assert 'blu2usb_hid_aggregator_release_source(aggregator, shortcut_source())' in core
assert 'BLU2USB_USB_HID_INTERFACE_COUNT 2u' in descriptor
assert 'BLU2USB_USB_HID_KEYCODE_COUNT 6u' in descriptor
assert 'blu2usb_modifier_shortcuts_process_event(' in main
assert 'blu2usb_modifier_shortcuts_ack_usb(&shortcuts, keyboard_accepted)' in main
assert 'blu2usb_modifier_shortcuts_assignments(shortcuts, profiles->shortcuts)' in main
assert 'state->assignments[combo]' in core
assert 'id == 1u || id == 7u' not in core
assert '[19]' not in catalog and '[24]' not in catalog
assert '[29]' not in catalog and '[30]' not in catalog
assert 'KEY_EQUAL' in catalog and 'KEY_MINUS' in catalog
assert 'KEY_HOME' not in catalog
assert 'KEY_PRINT_SCREEN, MOD_SHIFT | MOD_META' in catalog
print("MOD-07 21 direct Keyboard mappings plus Android Consumer AC Home and USB ACK runtime wiring PASS")
