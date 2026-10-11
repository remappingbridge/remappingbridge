#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(sys.argv[1] if len(sys.argv)>1 else ".").resolve()
h=(root/"include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ux=(root/"src/ux_model/ux_model.c").read_text(encoding="utf-8")
renderer=(root/"src/renderer/renderer.c").read_text(encoding="utf-8")
app=(root/"src/app/main.c").read_text(encoding="utf-8")
expected=[
    ("MODIFIER RELEASED IS","MOD + MIDDLE IS"),
    ("MOD + LEFT CLICK IS","MOD + RIGHT CLICK IS"),
    ("MOD + FORWARD IS","MOD + BACKWARD IS"),
    ("MOD + WHEEL UP IS","MOD + WHEEL DOWN IS"),
    ("MOD + MOVE LEFT IS","MOD + MOVE RIGHT IS"),
    ("MOD + MOVE UP IS","MOD + MOVE DOWN IS"),
]
for i,(left,right) in enumerate(expected,1):
    prefix=f'[BLU2USB_SCREEN_COMBINATIONS_{i}] = {{'
    assert prefix in ux
    screen=ux.split(prefix,1)[1].split("\n",1)[0]
    assert f'"COMBINATIONS {i} OF 6"' in screen
    assert f'" {left}"' in screen
    assert f'" {right}"' in screen
    assert screen.count('" NOT CUSTOMIZED"')==2
    assert r'JOY LEFT/RIGHT: PAGE' in screen
    assert '"JOY PRESS: ACCESS"' in screen
    assert '"KEY A: RESET SELECTED"' in screen
    assert "DYN(2)|DYN(4)" in screen
assert "BLU2USB_MOD_COMBO_COUNT" in h
assert "return BLU2USB_COMBINATIONS_PER_PAGE;" in ux
assert "BLU2USB_SCREEN_COMBINATIONS_1 + next" in ux
assert "ux->selection = 2u; /* Restore MODIFIER SHORTCUTS entry focus. */" in ux
assert "BLU2USB_UX_COMMAND_RESET_SHORTCUT" in ux
assert "BLU2USB_UX_COMMAND_SET_SHORTCUT" in ux
assert "emphasize_row(frame, (uint8_t)(selected + 1))" in renderer
assert "BLU2USB_SCREEN_COMBINATIONS_1" not in app, "MOD-02 must not touch BLE/HID runtime"
assert "MOD + WHELL" not in ux
assert "COMBINATIONS 1 0F 6" not in ux
print("MOD-02 6 pages/12 combos, 9x21, no HID/storage invariants: OK")
