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
assert "blu2usb_profiles_restore_any" in app
profiles=(root/"src/profiles/profiles.c").read_text(encoding="utf-8")
assert "BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP" in profiles
assert "BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP" in profiles
assert "candidate.active_kind = BLU2USB_MOUSE_PROFILE_PASSTHROUGH" in profiles
print("MOD-01 consolidated remapping flow contracts: OK")
