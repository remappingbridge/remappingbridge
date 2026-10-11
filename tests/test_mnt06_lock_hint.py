from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=(root/"src/ux_model/ux_model.c").read_text()
line=next(l for l in source.splitlines() if "[BLU2USB_SCREEN_MOUSE_SAVED] =" in l)
assert '"FIRST MOUSE CONNECTED"' in line
assert '"KEY Y: LOCKS ANYWHERE"' in line
assert '" KEY Y: LOCK"' not in line
assert len("KEY Y: LOCKS ANYWHERE") == 21
assert line.count('"')//2 == 9
assert 'if (event.kind == BLU2USB_INTERACTION_UNLOCK)' in source
assert 'enter(ux, BLU2USB_SCREEN_HOME);' in source
print("MNT-06 First Mouse Connected lock hint: PASS")
