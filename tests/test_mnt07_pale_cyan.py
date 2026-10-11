from pathlib import Path
root=Path(__file__).resolve().parents[1]
header=(root/"include/blu2usb/renderer/renderer.h").read_text()
renderer=(root/"src/renderer/renderer.c").read_text()
palette={
"BLACK":"0x0000u","WHITE":"0xffffu","MAGENTA":"0xf81fu",
"CYAN":"0xb65au","OFF_WHITE_YELLOW":"0xffb8u",
"LIGHT_GRAY":"0xc618u","DARK_MAGENTA":"0x0801u"
}
for name,value in palette.items():
    assert f"#define BLU2USB_COLOR_{name} {value}" in header,(name,value)
assert "case BLU2USB_UI_TONE_CURRENT: return BLU2USB_COLOR_CYAN;" in renderer
assert "case BLU2USB_UI_TONE_STATIC: return BLU2USB_COLOR_OFF_WHITE_YELLOW;" in renderer
assert "case BLU2USB_UI_TONE_ACTIONABLE: return BLU2USB_COLOR_LIGHT_GRAY;" in renderer
assert "case BLU2USB_UI_TONE_EMPHASIZED: return BLU2USB_COLOR_WHITE;" in renderer
# 0xB65A: R=22/31, G=50/63, B=26/31; muted gray-cyan versus bright 0xBFFF.
rgb=0xb65a
assert (rgb>>11, (rgb>>5)&63, rgb&31)==(22,50,26)
print("MNT-07 semantic pale-cyan palette contract PASS")
