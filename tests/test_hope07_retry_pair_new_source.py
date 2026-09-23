from pathlib import Path

root = Path(__file__).resolve().parents[1]
app = (root / "src/app/main.c").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_RETRY_PAIR_NEW" in ux_h
assert '"NEW MOUSE NOT FOUND"' in ux
assert '"NO NEW MOUSE OUTSIDE"' in ux
assert '"KEY A: RETRY NEW PAIR"' in ux
assert '"KEY B: BACK TRY SAVED"' in ux

# Timeout must stop showing the active Pair New presentation.
timeout_case = """case BLU2USB_BLE_HOGP_EVENT_PAIR_NEW_TIMEOUT:
            if (ux != NULL && ux->screen == BLU2USB_SCREEN_PAIR_MOUSE) {
                ux->screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;"""
assert timeout_case in app

# A retry enters active Pair New. The accepted HOPE-06 screen-transition hook
# must then request a new 15-second Pair New operation.
retry_transition = """if (ux->screen == BLU2USB_SCREEN_RETRY_PAIR_NEW &&
        control == BLU2USB_CONTROL_KEY_A) {
        enter(ux, BLU2USB_SCREEN_PAIR_MOUSE);"""
assert retry_transition in ux
request_hook = """screen_before != BLU2USB_SCREEN_PAIR_MOUSE &&
                ux.screen == BLU2USB_SCREEN_PAIR_MOUSE"""
assert request_hook in app
assert "blu2usb_ble_hogp_pico_request_pair_new();" in app

# Pair New Help now returns to retry, never to the active searching screen.
assert "ux->return_screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;" in ux

# HOPE-27 is still future.
assert "DEVICE NOT FOUND HELP" not in ux

print("HOPE-07 retry-pair-new source invariants: OK")
