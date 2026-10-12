from pathlib import Path
root=Path(__file__).resolve().parents[1]
header=(root/"include/blu2usb/renderer/renderer.h").read_text()
renderer=(root/"src/renderer/renderer.c").read_text()
palette={
"BLACK":"0x0000u","WHITE":"0xffffu","MAGENTA":"0xf81fu",
"CYAN":"0x07ffu","OFF_WHITE_YELLOW":"0xffb8u",
"LIGHT_GRAY":"0xc618u","DARK_MAGENTA":"0x0801u",
"CYAN_FOCUSED":"0xd7ffu"
}
for name,value in palette.items():
    assert f"#define BLU2USB_COLOR_{name} {value}" in header,(name,value)
assert "case BLU2USB_UI_TONE_CURRENT: return BLU2USB_COLOR_CYAN;" in renderer
assert "case BLU2USB_UI_TONE_SELECTED_CURRENT: return BLU2USB_COLOR_CYAN_FOCUSED;" in renderer
assert "case BLU2USB_UI_TONE_STATIC: return BLU2USB_COLOR_OFF_WHITE_YELLOW;" in renderer
assert "case BLU2USB_UI_TONE_ACTIONABLE: return BLU2USB_COLOR_LIGHT_GRAY;" in renderer
assert "case BLU2USB_UI_TONE_EMPHASIZED: return BLU2USB_COLOR_WHITE;" in renderer
# MNT-07 validated the pastel 0xD7FF token. MNT-08 makes it the focus color,
# leaving normal/current as traditional cyan 0x07FF, per operator request.
rgb=0xd7ff
assert (rgb>>11, (rgb>>5)&63, rgb&31)==(26,63,31)
previous_pale = 0xbfff
assert ((rgb >> 11) & 31) > ((previous_pale >> 11) & 31)
assert ((rgb >> 5) & 63) == ((previous_pale >> 5) & 63)
assert (rgb & 31) == (previous_pale & 31)
assert rgb != 0xc618  # System light gray remains a different token.
print("MNT-07 pale cyan preserved for MNT-08 selected state: PASS")
