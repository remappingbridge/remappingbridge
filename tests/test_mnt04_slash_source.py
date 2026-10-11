"""MNT-04: only display separators change; avoid touching C escapes and glyph."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
ux = (root / "src/ux_model/ux_model.c").read_text()
renderer = (root / "src/renderer/renderer.c").read_text()
expected = (
    "JOY UP/DOWN: SELECT", "JOY RIGHT/LEFT: PAGE",
    "JOY LEFT/RIGHT: PAGE", " ENTER/RETURN",
    " PLAY/PAUSE", " HOME SCREEN/PAGE",
)
for item in expected:
    assert item in ux, item
for old in ("JOY UP\\\\DOWN", "JOY RIGHT\\\\LEFT", "JOY LEFT\\\\RIGHT",
            "ENTER\\\\RETURN", "PLAY\\\\PAUSE", "HOME SCREEN\\\\PAGE"):
    assert old not in ux, old
assert 'strstr(row, "RIGHT/LEFT")' in renderer
assert "case '\\\\': return" in renderer
# All literal template strings must fit a 9x21 LCD, with no new overflow.
import re
for line in ux.splitlines():
    if line.strip().startswith("[BLU2USB_SCREEN_") and "={{" in line:
        head = line.split("={{", 1)[1].split("},", 1)[0]
        literals = re.findall(r'"([^"]*)"', head)
        assert len(literals) <= 9, (line[:60], len(literals))
        for literal in literals:
            assert len(literal) <= 21, (literal, len(literal))
print("MNT-04 LCD slash source contract PASS")
