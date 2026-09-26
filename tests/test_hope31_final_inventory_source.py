from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
header = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
inventory = (root / "docs/hope/00-final-screen-inventory.md").read_text(encoding="utf-8")
architecture = (root / "docs/hope/01-final-architecture.md").read_text(encoding="utf-8")
navigation = (root / "docs/hope/02-final-navigation.md").read_text(encoding="utf-8")

runtime = "\n".join((header, ux, app, renderer))

for forbidden in (
    "BLU2USB_SCREEN_MOUSE_STATUS",
    "BLU2USB_SCREEN_OTHER_DEVICES_STATUS",
    "BLU2USB_SCREEN_MOUSE_HELP",
    "BLU2USB_SCREEN_DEVICES_HELP",
    "BLU2USB_SCREEN_CUSTOM_APPLIED",
    "BLU2USB_SCREEN_LEARN_KEYS",
):
    assert forbidden not in runtime, forbidden

assert "unsigned status_page;" not in header
assert "status_page" not in ux
assert "BLU2USB_SCREEN_SEARCHING_FIRST" in header
assert '[BLU2USB_SCREEN_SEARCHING_FIRST] = {{"SEARCHING FIRST MOUSE"' in ux
assert "else screen = BLU2USB_SCREEN_SEARCHING_FIRST;" in ux

enum_match = re.search(
    r"typedef enum \{(.*?)BLU2USB_SCREEN_COUNT\s*\n\} blu2usb_screen_id_t;",
    header,
    re.S,
)
assert enum_match
ids = re.findall(r"BLU2USB_SCREEN_[A-Z0-9_]+", enum_match.group(1))
assert len(ids) == 29, ids
assert len(set(ids)) == 29

assert "29 canonical runtime screens" in inventory
assert "HOPE-23 intentionally contributes **no screen**" in inventory
assert "MOUSE_STATUS" in inventory and "Removed final-inventory residues" in inventory
assert "architecture that actually exists" in architecture
assert "There is no user-facing Keyboard or Composite pairing flow" in architecture
assert "cancelled Learn The Keys option does not exist" in navigation

print("HOPE-31 final inventory/source/documentation invariants: OK")
