from pathlib import Path
root = Path(__file__).resolve().parents[1]
ble = (root/"src/ble_hogp/ble_hogp_pico.c").read_text()
hdr = (root/"include/blu2usb/ble_hogp/ble_hogp.h").read_text()
app = (root/"src/app/main.c").read_text()
assert "BLE_HOGP_PROFILE_TAG" in ble
assert "BLE_HOGP_PROFILE_VERSION" in ble
assert "BLE_HOGP_PROFILE_ENTRY_SIZE" in ble
assert "g_profiles_have_registry" in ble
assert "profiles_seed_legacy()" in ble
assert "bond_slot_is_first_for_identity(slot)" in ble
assert "bond_identity_equal(" in ble
assert "profiles_remove_identity(g_remove_target_type" in ble
assert "movements_remove_identity(g_remove_target_type" in ble
assert "saved_names_remove_identity(g_remove_target_type" in ble
removal = ble.split("static void complete_remove_target(void)\n{",1)[1].split("\nstatic void dedupe_current_bond",1)[0]
assert removal.index("profiles_remove_identity(") < removal.index("movements_remove_identity(")
assert removal.index("movements_remove_identity(") < removal.index("saved_names_remove_identity(")
assert removal.index("saved_names_remove_identity(") < removal.index("remove_bonds_for_identity(")
assert removal.index("remove_bonds_for_identity(") < removal.index("BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVED")
assert removal.count("BLE_HOGP_MESSAGE_SAVED_MOUSE_REMOVE_FAILED") >= 3
assert "g_profiles_corrupt" in ble
assert "if (g_profiles_have_registry) return true" in ble
assert "if (n <= 0)" in ble
assert "blu2usb_ble_hogp_pico_current_profile(" in hdr
assert "blu2usb_ble_hogp_pico_set_current_profile(" in hdr
assert "blu2usb_ble_hogp_pico_set_legacy_profiles(" in hdr
assert "blu2usb_ble_hogp_pico_set_legacy_profiles(legacy)" in app
assert "blu2usb_ble_hogp_pico_set_current_profile(payload)" in app
assert "reload_session_profiles(ux, profiles, shortcuts, remap," in app
assert "blu2usb_storage_store(payload" not in app
assert "blu2usb_profiles_init(&next);" in app
assert "synchronize_ux_profiles(ux, profiles);" in app
assert "blu2usb_modifier_shortcuts_configure(shortcuts, &cfg," in app
assert "blu2usb_remap_set_profile(remap, &cfg);" in app
assert "if (connected && blu2usb_ble_hogp_pico_current_profile(payload))" in app
print("MNT-10 profile migration and bond-scoped purge source contract PASS")
