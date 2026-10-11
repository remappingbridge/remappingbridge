from pathlib import Path
r=Path(__file__).resolve().parents[1]
descriptor=(r/"src/usb_hid/usb_descriptors.c").read_text()
pico=(r/"src/usb_hid/usb_hid_pico.c").read_text()
header=(r/"include/blu2usb/usb_hid/usb_hid.h").read_text()
app=(r/"src/app/main.c").read_text()
core=(r/"src/modifier_shortcuts/modifier_shortcuts.c").read_text()
assert "TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(BLU2USB_USB_REPORT_ID_KEYBOARD))" in descriptor
assert "TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(BLU2USB_USB_REPORT_ID_CONSUMER))" in descriptor
assert "HID_ITF_PROTOCOL_KEYBOARD" in descriptor
assert "BLU2USB_USB_HID_INTERFACE_COUNT 2u" in header
assert "BLU2USB_USB_HID_MOUSE_INTERFACE 0u" in header
assert "BLU2USB_USB_HID_KEYBOARD_INTERFACE 1u" in header
assert "CFG_TUD_HID 2" in (r/"include/tusb_config.h").read_text()
assert "BLU2USB_USB_REPORT_ID_CONSUMER" in pico
assert "tud_hid_n_get_protocol" in pico and "HID_PROTOCOL_BOOT" in pico
assert "tud_hid_n_report(" in pico
assert "const uint8_t report[BLU2USB_USB_CONSUMER_PAYLOAD_SIZE]" in pico
assert "last_consumer_sent" in app and "blu2usb_usb_hid_pico_send_consumer(0u)" in app
assert "blu2usb_modifier_shortcuts_consumer_phase(&shortcuts)" in app
assert "blu2usb_modifier_shortcuts_ack_usb(&shortcuts, accepted)" in app
assert "blu2usb_modifier_shortcuts_ack_usb(&shortcuts, keyboard_accepted)" in app
assert "blu2usb_consumer_shortcut_lookup" in core
assert "state->consumer_usage = 0u" in core
assert "state->running_consumer = false" in core
assert "CFG_TUD_CDC 0" in (r/"include/tusb_config.h").read_text()
assert "CFG_TUD_HID_EP_BUFSIZE 16" in (r/"include/tusb_config.h").read_text()
assert "last_consumer_sent != 0u &&" in app
assert "blu2usb_usb_hid_pico_consumer_available()) {" in app
assert "state->running_consumer" in core
print("MOD08 combined two-interface HID ReportID + Boot legacy + neutralize/cancel source PASS")
