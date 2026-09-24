from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
feedback = (root / "src/renderer/profile_feedback.c").read_text(encoding="utf-8")

canonical = '{"REMAPPING OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP"," CUSTOM REMAP",EMPTY,"JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"}'
assert canonical in ux

# Old visual is replaced in-place.
assert '{"MOUSE OPTIONS"," PAIR MOUSE"," PASSTHROUGH"," DEFAULT REMAP"' not in ux
assert "case BLU2USB_SCREEN_MOUSE_OPTIONS: return 4;" in ux

# Pair New is not an option here anymore.
start = ux.index("if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS && control == BLU2USB_CONTROL_JOY_PRESS)")
end = ux.index("if (ux->screen == BLU2USB_SCREEN_EDIT_CUSTOM", start)
block = ux[start:end]
assert "BLU2USB_SCREEN_PAIR_MOUSE" not in block
assert "BLU2USB_UX_COMMAND_PAIR_MOUSE" not in block

# Current Mouse UI v1 profile ordering maps active cyan to rows 1..4.
for token in (
    "BLU2USB_MOUSE_PROFILE_PASSTHROUGH: return 1u",
    "BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP: return 2u",
    "BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP: return 3u",
    "BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP: return 4u",
):
    assert token in feedback

# The legacy connected-mouse cyan row 1 special-case is gone; row 1 now means
# PASSTHROUGH and is cyan only when passthrough is the active profile.
assert "ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS && blu2usb_ux_mouse_connected()" not in feedback

# HOPE-29 is bundled into this consolidated remapper-flow candidate.
assert '"REMAPPER OPTIONS HELP"' in ux
assert "BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS" in ux

# Exclusive-current rendering must rebuild all option tones before applying
# authoritative active cyan, then selected white.
assert "for (uint8_t row = 1u; row <= 4u; ++row)" in feedback
assert "set_row_tone(frame, row, BLU2USB_UI_TONE_ACTIONABLE);" in feedback
assert "set_row_tone(frame, profile_row, BLU2USB_UI_TONE_CURRENT);" in feedback
assert "BLU2USB_UI_TONE_EMPHASIZED" in feedback

print("HOPE-10 remapper-options source invariants: OK")

# Bundled v1 visible names replace legacy strings.
for old in (
    '"PASSTHROUGH APPLIED"',
    '"APPLY DEFAULT REMAP"',
    '"DEFAULT REMAP APPLIED"',
    '"APPLY ESCAPE"',
    '"ESCAPE APPLIED"',
    '"CUSTOM APPLIED"',
):
    assert old not in ux

for new in (
    '"PASSTHROUGH ACTIVE"',
    '"APPLY STANDARD REMAP"',
    '"STANDARD REMAP ACTIVE"',
    '"APPLY ESCAPE REMAP"',
    '"ESCAPE APPLIED ACTIVE"',
    '"EDIT CUSTOM REMAP"',
):
    assert new in ux
