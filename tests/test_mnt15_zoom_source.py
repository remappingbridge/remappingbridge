#!/usr/bin/env python3
"""MNT-15: lock the sole generic Zoom In HID mapping without app detection."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
catalog = (root / "src/modifier_shortcuts/keyboard_catalog.c").read_text()
assert "[26] = {KEY_EQUAL, MOD_CTRL}," in catalog
assert "[26] = {KEY_EQUAL, MOD_CTRL | MOD_SHIFT}" not in catalog
assert "[27] = {KEY_MINUS, MOD_CTRL}," in catalog
assert "[28] = {KEY_0, MOD_CTRL}," in catalog
assert "[6] = {KEY_Z, MOD_CTRL | MOD_SHIFT}" in catalog
assert "[15] = {KEY_PRINT_SCREEN, MOD_SHIFT | MOD_META}" in catalog
assert "KEY_EQUAL = 0x2eu" in catalog
assert "KEY_MINUS = 0x2du" in catalog
assert "KEY_0 = 0x27u" in catalog

test = (root / "tests/test_mod07_keyboard_catalog.c").read_text()
assert "{26, 0x2e, 0x01}," in test
assert "{27, 0x2d, 0x01}, {28, 0x27, 0x01}" in test
print("MNT-15 generic zoom HID source contract passed")
