"""MNT-13 source contract: the final overlay must execute on physical frames."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
app = (ROOT / "src/app/main.c").read_text(encoding="utf-8")
renderer = (ROOT / "src/renderer/renderer.c").read_text(encoding="utf-8")
header = (ROOT / "include/blu2usb/renderer/renderer.h").read_text(encoding="utf-8")

render_state = app.split("static bool render_state(", 1)[1].split(
    "static int8_t clamp_i8(", 1
)[0]
stages = (
    "blu2usb_ui_project(ux, &frame);",
    "blu2usb_ui_enforce_applied_visual_contract(ux, &frame);",
    "blu2usb_ui_apply_selection_marker(ux, &frame);",
    "blu2usb_renderer_render(display, &frame);",
)
positions = [render_state.index(stage) for stage in stages]
assert positions == sorted(positions), "Final selection overlay must follow both color passes"
assert "case '|': return 0x04u;" in renderer, "5x7 pipe glyph must exist for LCD hardware"
assert "void blu2usb_ui_apply_selection_marker(" in header
assert "ux->selection < count" in renderer, "Reject invalid selections"
assert "blu2usb_ux_saved_bond_for_page(ux, ux->saved_page) >= 0" in renderer
assert "frame->cells[row][0].tone = marker_tone;" in renderer
assert "marker_tone = frame->cells[row - 1][1].tone;" in renderer
assert "frame->cells[row][0].character = '|';" in renderer
assert "if (blu2usb_ux_is_combinations_screen(ux->screen)) ++row;" in renderer
assert "row = 5; /* Single implicit REMOVE DEVICE action. */" in renderer
assert "marker_tone == BLU2USB_UI_TONE_STATIC" in renderer
print("MNT-13: production frame pipeline, first-column glyph, tone and guards PASS")
