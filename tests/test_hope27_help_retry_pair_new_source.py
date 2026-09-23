from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW" in ux_h
assert '"DEVICE NOT FOUND HELP"' in ux
assert '"KEY B TO BACK UNTIL"' in ux
assert '"ANY KEY: BACK"' in ux

# The explanatory KEY-prefixed prose must never drive hint detection.
assert "ux->screen == BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW" in renderer
assert "BLU2USB_SCREEN_HELP_PAIR_NEW ||" in renderer

# Retry X opens Help and the Help return target is retry itself.
assert "enter(ux, BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);" in ux
assert "ux->return_screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;" in ux

# There must be no app lifecycle hook for this Help: retry has no active Pair New
# operation, and returning to retry must not enter active Pair New.
assert "BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW" not in app

# A remains the only retry action that enters active Pair New.
retry_a = """if (ux->screen == BLU2USB_SCREEN_RETRY_PAIR_NEW &&
        control == BLU2USB_CONTROL_KEY_A) {
        enter(ux, BLU2USB_SCREEN_PAIR_MOUSE);"""
assert retry_a in ux

print("HOPE-27 help-retry-pair-new source invariants: OK")
