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

# HOPE-07 now owns the post-Help retry state. HOPE-26 must continue to
# prove that Help cancellation does not itself restart Pair New.
assert "BLU2USB_SCREEN_RETRY_PAIR_NEW" in ux_h
assert "NO NEW MOUSE OUTSIDE" in ux

# HOPE-27 remains future.
assert "DEVICE NOT FOUND HELP" not in ux

print("HOPE-26 help-pair-new source invariants: OK")


# Pair New Help contains prose beginning with "KEY B TO BACK UNTIL"; that is
# explanatory body text, not a hint. The renderer must explicitly start the
# hint region only at the final ANY KEY row.
renderer = (root / "src/renderer/renderer.c").read_text(encoding="utf-8")
assert "ux->screen == BLU2USB_SCREEN_HELP_PAIR_NEW" in renderer
assert "? 8u" in renderer
