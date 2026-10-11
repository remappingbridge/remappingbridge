from pathlib import Path
root=Path(__file__).resolve().parents[1]
ux=(root/"src/ux_model/ux_model.c").read_text(encoding="utf-8")
renderer=(root/"src/renderer/renderer.c").read_text(encoding="utf-8")
feedback=(root/"src/renderer/profile_feedback.c").read_text(encoding="utf-8")
app=(root/"src/app/main.c").read_text(encoding="utf-8")
assert '"REMAPPING OPTIONS"," PASSTHROUGH DEFAULT"," CUSTOM REMAPPING"," MODIFIER SHORTCUTS"' in ux
assert 'BLU2USB_SCREEN_EDIT_CUSTOM' in ux
assert 'BLU2USB_SCREEN_PASSTHROUGH_APPLIED' in ux
assert 'BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS' in ux
assert '"REMAPPING OPTIONS HELP"' in ux
assert "ux->selection = selection % 3u;" in ux
assert "for (uint8_t row = 1u; row <= 3u; ++row)" in feedback
assert "BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP: return 2u;" in feedback
assert '" REMAPPED TO STANDARD"' not in renderer
assert '" REMAPPED TO ESCAPE"' not in renderer
assert 'return "STANDARD";' not in renderer
assert 'return "ESCAPE";' not in renderer
assert "profiles.active_kind = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;" in app
# Legacy enum names survive only for v2-compatible deserialization and future MOD-03 migration.
assert "BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP" in app
assert "BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP" in app
print("MOD-01 consolidated remapping flow contracts: OK")
