from pathlib import Path
root=Path(__file__).resolve().parents[1]
app=(root/"src/app/main.c").read_text()
core=(root/"src/modifier_shortcuts/modifier_shortcuts.c").read_text()
move=(root/"src/movement/movement.c").read_text()
api=(root/"include/blu2usb/modifier_shortcuts/modifier_shortcuts.h").read_text()
assert "blu2usb_modifier_shortcuts_process_event(" in app
assert "blu2usb_modifier_shortcuts_tick(&shortcuts" in app
assert "blu2usb_remap_process_mouse(remap, &forwarded, &mapped)" in app
assert "blu2usb_movement_rotate_delta" in core
assert "blu2usb_movement_rotate_delta(settings, source_x, source_y" in move
assert "BLU2USB_MOD06_QUIET_MS" in core
assert "BLU2USB_MOD06_MOVE_THRESHOLD 24" in api
assert "state->gesture_latched = true" in core
assert "forwarded->data.wheel.vertical = 0" in core
assert "return forwarded->data.wheel.horizontal != 0" in core
assert "state->assignments[combo]" in core
assert "blu2usb_modifier_shortcuts_cancel(&shortcuts, &aggregator)" in app
print("MOD-06 rotated pre-gain gesture/wheel before remap, no double feed PASS")
