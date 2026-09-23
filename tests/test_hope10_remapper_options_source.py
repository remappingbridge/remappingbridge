from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
feedback = (root / "src/renderer/profile_feedback.c").read_text(encoding="utf-8")

canonical = '{"MOUSE OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP"," CUSTOM REMAP",EMPTY,"JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"}'
assert canonical in ux

# Old visual is replaced in-place.
assert '{"MOUSE OPTIONS"," PAIR MOUSE"," PASSTHROUGH"," DEFAULT REMAP"' not in ux
assert "case BLU2USB_SCREEN_MOUSE_OPTIONS: return 4;" in ux

# Pair New is not an option here anymore.
start = ux.index("if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS && control == BLU2USB_CONTROL_JOY_PRESS)")
end = ux.index("if (ux->screen == BLU2USB_SCREEN_OTHER_OPTIONS", start)
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

# HOPE-29 is still future.
assert "HOPE-29 owns help-remapper-options" in ux

print("HOPE-10 remapper-options source invariants: OK")
