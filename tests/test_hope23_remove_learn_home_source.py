from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")

home_lines = [
    line for line in ux.splitlines()
    if "[BLU2USB_SCREEN_HOME]" in line
    or "[BLU2USB_SCREEN_HOME_SEARCHING]" in line
    or "[BLU2USB_SCREEN_HOME_RETRY]" in line
]
assert len(home_lines) == 3
assert all("LEARN THE KEYS" not in line for line in home_lines)

assert "case BLU2USB_SCREEN_HOME: return 3;" in ux
assert "case BLU2USB_SCREEN_HOME_RETRY: return 2;" in ux
assert "selection % 3u" in ux

# HOME destinations expose only remapper, saved devices and Pair New.
assert "static const blu2usb_screen_id_t dest[3]" in ux
assert "static const blu2usb_screen_id_t dest[2]" in ux

home_nav_start = ux.index(
    "if (ux->screen == BLU2USB_SCREEN_HOME && control == BLU2USB_CONTROL_JOY_PRESS)"
)
home_nav_end = ux.index(
    "if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS", home_nav_start
)
home_nav = ux[home_nav_start:home_nav_end]
assert "BLU2USB_SCREEN_SEARCHING_FIRST" not in home_nav

# Searching First is preserved as the automatic zero-saved-device resolver,
# not as a user-facing HOME option.
assert "else screen = BLU2USB_SCREEN_SEARCHING_FIRST;" in ux
assert '[BLU2USB_SCREEN_SEARCHING_FIRST] = {{"SEARCHING FIRST MOUSE"' in ux

print("HOPE-23 remove learn-the-keys HOME option invariants: OK")
