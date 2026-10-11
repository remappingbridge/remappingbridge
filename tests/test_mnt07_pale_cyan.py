from pathlib import Path
root=Path(__file__).resolve().parents[1]
header=(root/"include/blu2usb/renderer/renderer.h").read_text()
renderer=(root/"src/renderer/renderer.c").read_text()
palette={
"BLACK":"0x0000u","WHITE":"0xffffu","MAGENTA":"0xf81fu",
"CYAN":"0xbfffu","OFF_WHITE_YELLOW":"0xffb8u",
"LIGHT_GRAY":"0xc618u","DARK_MAGENTA":"0x0801u"
}
for name,value in palette.items():
    assert f"#define BLU2USB_COLOR_{name} {value}" in header,(name,value)
assert "case BLU2USB_UI_TONE_CURRENT: return BLU2USB_COLOR_CYAN;" in renderer
assert "case BLU2USB_UI_TONE_STATIC: return BLU2USB_COLOR_OFF_WHITE_YELLOW;" in renderer
assert "case BLU2USB_UI_TONE_ACTIONABLE: return BLU2USB_COLOR_LIGHT_GRAY;" in renderer
assert "case BLU2USB_UI_TONE_EMPHASIZED: return BLU2USB_COLOR_WHITE;" in renderer
# 0xBFFF: R=23/31, G=63/63, B=31/31, visibly paler than prior R=0/31.
rgb=0xbfff
assert (rgb>>11, (rgb>>5)&63, rgb&31)==(23,63,31)
print("MNT-07 semantic pale-cyan palette contract PASS")
