from pathlib import Path

root = Path(__file__).resolve().parents[1]
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_HELP_PAIR_NEW" in ux_h
assert '"PAIR NEW DEVICE HELP"' in ux
assert '"TO CONNECT A SAVED"' in ux
assert '"SEARCHING APPEARS."' in ux

# Entering Help moves away from Pair New, so the accepted HOPE-06 cancel hook
# must still cancel the active candidate search.
cancel_guard = """if (screen_before == BLU2USB_SCREEN_PAIR_MOUSE &&
                (ux.screen != BLU2USB_SCREEN_PAIR_MOUSE ||
                 (!was_locked && is_locked)))"""
assert cancel_guard in app
assert "blu2usb_ble_hogp_pico_cancel_pair_new();" in app

# Returning from Help uses Pair New only as the temporary HOPE-07 retry
# placeholder. It must not request a fresh 15-second Pair New operation.
request_guard = """screen_before != BLU2USB_SCREEN_PAIR_MOUSE &&
                screen_before != BLU2USB_SCREEN_HELP_PAIR_NEW &&
                ux.screen == BLU2USB_SCREEN_PAIR_MOUSE"""
assert request_guard in app

# HOPE-07 / HOPE-27 visuals must not be introduced early.
assert "NO NEW MOUSE OUTSIDE" not in ux
assert "DEVICE NOT FOUND HELP" not in ux

print("HOPE-26 help-pair-new source invariants: OK")
