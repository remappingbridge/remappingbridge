from pathlib import Path
root = Path(__file__).resolve().parents[1]
ux = (root/"src/ux_model/ux_model.c").read_text(encoding="utf-8")
feedback = (root/"src/renderer/profile_feedback.c").read_text(encoding="utf-8")
renderer = (root/"src/renderer/renderer.c").read_text(encoding="utf-8")
app = (root/"src/app/main.c").read_text(encoding="utf-8")
assert '"REMAPPING OPTIONS"," PASSTHROUGH DEFAULT"," CUSTOM REMAPPING"," MODIFIER SHORTCUTS"' in ux
assert 'case BLU2USB_SCREEN_MOUSE_OPTIONS: return 3;' in ux
assert '"REMAPPING OPTIONS HELP","PASSTHROUGH DEFAULT"' in ux
assert "ux->selection = selection % 3u;" in ux
assert "for (uint8_t row = 1u; row <= 3u; ++row)" in feedback
assert "BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP: return 2u;" in feedback
assert "if (ux->selection < 3u)" in feedback
assert '" REMAPPED TO STANDARD"' not in renderer
assert '" REMAPPED TO ESCAPE"' not in renderer
assert 'return "STANDARD";' not in renderer
assert 'return "ESCAPE";' not in renderer
assert "profiles.active_kind = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;" in app
assert "if (profiles.active_kind == BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP ||" in app
start=ux.index("if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS && control == BLU2USB_CONTROL_JOY_PRESS)")
end=ux.index("if (ux->screen == BLU2USB_SCREEN_EDIT_CUSTOM",start)
flow=ux[start:end]
assert "BLU2USB_SCREEN_APPLY_DEFAULT" not in flow
assert "BLU2USB_SCREEN_APPLY_ESCAPE" not in flow
assert "BLU2USB_SCREEN_EDIT_CUSTOM" in flow
assert "MOD-02" in flow
print("MOD-01 remapping-options source contracts: OK")
