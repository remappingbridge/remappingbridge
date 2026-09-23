from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
app = (root / "src/app/main.c").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_HELP_HOME_CONNECTED" in ux_h
assert '"HOME CONNECTED HELP"' in ux
assert '"MOUSE, NAVIGATE TO:"' in ux
assert '"(MOUSE PAGE) > REMOVE"' in ux
assert '"ANY KEY: BACK"' in ux

# Help owns Y through generic Help semantics and cannot reach global lock.
assert "screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED" in ux
assert "return screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED" not in ux
assert "ux->return_selection = ux->selection;" in ux
assert "enter(ux, BLU2USB_SCREEN_HELP_HOME_CONNECTED);" in ux

# HOME selection is explicitly restored on normal Help exit.
assert "const unsigned selection = ux->return_selection;" in ux
assert "ux->selection = selection % 4u;" in ux

# Disconnect keeps Help visible but retargets its return to home-searching.
disconnect = """else if (ux->screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED &&
                           ux->saved_device_count > 0u) {
                    /* Mouse UI v1 keeps Help visible on disconnect, but
                     * retargets Any-Key Back to home-searching. */
                    ux->return_screen = BLU2USB_SCREEN_HOME_SEARCHING;
                    ux->return_selection = 0u;"""
assert disconnect in app

# If saved reconnect completes before Help exit, the return target is restored
# to live HOME instead of sending a connected Mouse into stale searching UI.
assert "ux->return_screen == BLU2USB_SCREEN_HOME_SEARCHING" in app
assert "ux->return_screen = BLU2USB_SCREEN_HOME;" in app

# This gate is presentation/navigation only: it introduces no new UX command.
assert "BLU2USB_UX_COMMAND_HELP" not in ux_h

print("HOPE-28 help-home-connected source invariants: OK")
