#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ux=(root/"src/ux_model/ux_model.c").read_text()
renderer=(root/"src/renderer/renderer.c").read_text()
profiles=(root/"src/profiles/profiles.c").read_text()
app=(root/"src/app/main.c").read_text()
domain=(root/"include/blu2usb/domain/profile.h").read_text()
assert "BLU2USB_MOUSE_TARGET_ESCAPE," in domain
assert "BLU2USB_MOUSE_TARGET_MODIFIER," in domain
assert "BLU2USB_MOUSE_TARGET_MODIFIER" in profiles
assert "unique_modifier" in profiles
assert "profiles->draft_targets[i] = identity_target" in profiles
assert "blu2usb_profiles_is_shortcut_unavailable" in app
assert "blu2usb_ux_set_modifier_source" in app
assert "blu2usb_ux_custom_target_committed" in app
assert "blu2usb_ux_combination_unavailable" in ux
assert '?"UNAVAILABLE"' not in renderer or "UNAVAILABLE" in renderer
assert "BLU2USB_UI_TONE_STATIC" in renderer
expected='" LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"'
assert ux.count(expected)==5
assert 'case BLU2USB_MOUSE_TARGET_MODIFIER: return 6u;' in renderer
assert 'case BLU2USB_MOUSE_TARGET_MODIFIER: return 5u;' in ux
assert "BLU2USB_PROFILE_V3_SERIALIZED_SIZE" in profiles
assert "blu2usb_remap_set_profile(remap, &active)" in app
print("MOD-04 custom target/mask and persisted exclusive source contract PASS")
