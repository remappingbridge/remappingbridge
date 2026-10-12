from pathlib import Path
import sys
root=Path(sys.argv[1] if len(sys.argv)>1 else ".").resolve()
ux=(root/"src/ux_model/ux_model.c").read_text()
render=(root/"src/renderer/renderer.c").read_text()
profiles=(root/"src/profiles/profiles.c").read_text()
app=(root/"src/app/main.c").read_text()
header=(root/"include/blu2usb/ux_model/ux_model.h").read_text()
for i in range(1,6):
    assert f'BLU2USB_SCREEN_SHORTCUTS_{i}' in ux
assert "BLU2USB_PROFILE_V3_SERIALIZED_SIZE 26u" in (root/"include/blu2usb/profiles/profiles.h").read_text()
assert "PROFILE_V3_SCHEMA_VERSION 3u" in profiles
assert "blu2usb_profiles_restore_any" in profiles
assert "blu2usb_ble_hogp_pico_set_legacy_profiles" in app
assert "blu2usb_ble_hogp_pico_set_current_profile" in app
assert "blu2usb_storage_store(payload" not in app
assert "blu2usb_profiles_serialize_v3" in app
assert "BLU2USB_UX_COMMAND_SET_SHORTCUT" in app
assert "BLU2USB_UX_COMMAND_RESET_SHORTCUT" in app
assert "persist_profiles(profiles))" in app
assert "blu2usb_ux_shortcut_committed(ux, command.combination_id, id)" in app
assert "if (ux->screen" in ux
assert "blu2usb_ux_selected_shortcut" in header
assert 'blu2usb_ux_shortcut_name(ux->shortcut_assignments[combo])' in render
assert "const uint8_t assigned = ux->shortcut_assignments[ux->editing_combination]" in render
print("MOD-03 5 screen routes and v3 persistence static contracts PASS")
