from pathlib import Path

root = Path(__file__).resolve().parents[1]
app = (root / "src/app/main.c").read_text(encoding="utf-8")

needle = """case BLU2USB_BLE_HOGP_EVENT_SAVED_SEARCH_TIMEOUT:
            if (ux != NULL && ux->screen == BLU2USB_SCREEN_HOME_SEARCHING) {
                ux->screen = BLU2USB_SCREEN_HOME_RETRY;"""

assert needle in app, "saved-search timeout must resolve directly to HOME_RETRY"
assert "HOPE-09 will replace this temporary retry placeholder" not in app
