from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
feedback = (root / "src/renderer/profile_feedback.c").read_text(encoding="utf-8")
profile_state = (root / "src/ux_model/profile_state.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")

# Explicit remappingbridge improvement.
assert '"REMAPPING OPTIONS"' in ux
assert '[BLU2USB_SCREEN_MOUSE_OPTIONS] = {{"MOUSE OPTIONS"' not in ux

# Bundled HOPE-29 Help.
assert "BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS" in ux_h
assert '"REMAPPER OPTIONS HELP"' in ux
assert '"PASSTHROUGH IS THE","DEFAULT OPTION."' in ux

# Preset v1 titles.
for literal in (
    '"PASSTHROUGH ACTIVE"',
    '"APPLY PASSTHROUGH"',
    '"APPLY STANDARD REMAP"',
    '"STANDARD REMAP ACTIVE"',
    '"APPLY ESCAPE REMAP"',
    '"ESCAPE APPLIED ACTIVE"',
):
    assert literal in ux

for legacy in (
    '"PASSTHROUGH APPLIED"',
    '"APPLY DEFAULT REMAP"',
    '"DEFAULT REMAP APPLIED"',
    '"CUSTOM APPLIED"',
):
    assert legacy not in ux

# UI target order must not follow the legacy/domain enum order.
editor_order = '" LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD"'
assert ux.count(editor_order) == 5
assert "custom_target_for_selection" in ux
assert "custom_selection_for_target" in ux
assert "BLU2USB_MOUSE_TARGET_ESCAPE" in ux

# Custom success returns to Custom Edit rather than a legacy success page.
custom_case = """case BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP:
        /* Mouse UI v1 has no separate CUSTOM APPLIED screen."""
assert custom_case in profile_state
assert "ux->screen = BLU2USB_SCREEN_EDIT_CUSTOM;" in profile_state

# Exclusive cyan rebuild is deterministic every projection.
assert "for (uint8_t row = 1u; row <= 4u; ++row)" in feedback
assert "set_row_tone(frame, row, BLU2USB_UI_TONE_ACTIONABLE);" in feedback
assert "set_row_tone(frame, profile_row, BLU2USB_UI_TONE_CURRENT);" in feedback
assert "BLU2USB_UI_TONE_EMPHASIZED" in feedback

# Source editor current target uses explicit UI-row mapping.
assert "static uint8_t custom_target_row" in feedback
assert "BLU2USB_MOUSE_TARGET_ESCAPE: return 4u" in feedback
assert "BLU2USB_MOUSE_TARGET_BACKWARD: return 6u" in feedback

# Active preset screens demote if the live Mouse disconnects.
for line in (
    "ux->screen == BLU2USB_SCREEN_PASSTHROUGH_APPLIED",
    "ux->screen = BLU2USB_SCREEN_APPLY_PASSTHROUGH;",
    "ux->screen == BLU2USB_SCREEN_DEFAULT_APPLIED",
    "ux->screen = BLU2USB_SCREEN_APPLY_DEFAULT;",
    "ux->screen == BLU2USB_SCREEN_ESCAPE_APPLIED",
    "ux->screen = BLU2USB_SCREEN_APPLY_ESCAPE;",
):
    assert line in app

print("Consolidated REMAPPING OPTIONS flow source invariants: OK")
