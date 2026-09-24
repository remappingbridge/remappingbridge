from pathlib import Path

root = Path(__file__).resolve().parents[1]
ux_h = (root / "include/blu2usb/ux_model/ux_model.h").read_text(encoding="utf-8")
ux = (root / "src/ux_model/ux_model.c").read_text(encoding="utf-8")

assert "BLU2USB_SCREEN_HELP_REMOVE_THIS" in ux_h
assert '"REMOVE MOUSE HELP","COMPLETELY REMOVE THE","AUTOMATIC CONNECTION","WHEN TURNING ON THE","DEVICE AND DELETE ITS","BUTTON REMAPPING","PROFILE.",EMPTY,"ANY KEY: BACK"' in ux
assert "screen == BLU2USB_SCREEN_HELP_REMOVE_THIS" in ux
assert "case BLU2USB_SCREEN_HELP_REMOVE_THIS: return ux->return_screen;" in ux
assert "control == BLU2USB_CONTROL_KEY_X" in ux
assert "!ux->remove_pending" in ux
assert "ux->return_screen = BLU2USB_SCREEN_REMOVE_THIS;" in ux
assert "enter(ux, BLU2USB_SCREEN_HELP_REMOVE_THIS);" in ux

# Generic Help handling executes before global KEY Y lock handling, so every
# complete HAT control returns from Help and KEY Y cannot lock this screen.
help_pos = ux.index("if (is_help(ux->screen))")
lock_pos = ux.index("control == BLU2USB_CONTROL_KEY_Y && lock_allowed")
assert help_pos < lock_pos

# HOPE-30 must not alter the physical removal implementation.
assert "BLU2USB_UX_COMMAND_REMOVE_MOUSE" in ux_h
assert "cmd.saved_bond = ux->remove_target_bond;" in ux

print("HOPE-30 help-remove-this source invariants: OK")
