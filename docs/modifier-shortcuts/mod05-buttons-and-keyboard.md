# MOD-05 — Physical-button Modifier, Escape, Copy and USB handshake

**Gate candidate** from operator-accepted corrected MOD-04 commit `1147e79cb80d23412488cd10e92f14a9f75f26a5`, branch `feature/modifier-shortcuts-mod05`, review PR #25 draft. No merge/release; operator must physically accept this UF2 before MOD-06.

## End-to-end on Raspberry Pi Pico 2 W

HOME (BLE mouse connected) → REMAPPING OPTIONS → CUSTOM REMAPPING → LEFT WILL BECOME → MODIFIER → KEY A APPLY AND BACK → KEY A APPLY CUSTOM. Separately, REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS 1 OF 6 → MODIFIER RELEASED IS → ESCAPE (catalog ID 7) and COMBINATIONS 2 OF 6 → MOD + RIGHT CLICK IS → COPY (ID 1). KEY A APPLY AND BACK saves each.

While Custom with Modifier physically LEFT is applied:
- Press/release LEFT **alone** produces exactly one Escape keyboard usage 0x29 on release; a held button does not repeat and is not forwarded as LEFT Mouse click.
- Press and hold LEFT, press RIGHT: a single Left Ctrl + C chord (Linux desktop first target). RIGHT down/up are consumed, even if LEFT is released first. Release LEFT cannot append Escape after a chord.
- Press another (unassigned) mouse button while LEFT held: forward its ordinary Custom action, but suppress the solo-release Escape. An already held physical secondary button never spuriously triggers a chord; repeated pressed reports never produce a second action.
- No active Custom Modifier or Passthrough: Mouse works as before, 12 shortcuts remain stored but do not execute. Masked UNAVAILABLE combos do not trigger an event.
- Tiny pointer jitter under 8 relative raw counts per Modifier hold does not suppress a clean tap; accumulated movement reaching 8 raw counts suppresses it. This is **only a provisional anti-accidental-release budget**, not a MOVE classifier; MOD-06 will refine with physical testing. Wheel and MOVE reports continue to Mouse untransformed by this module until MOD-06, although vertical/horizontal wheel use prevents the solo Escape.

## USB behavior and safety
- The module `src/modifier_shortcuts/modifier_shortcuts.c` runs between canonical BLE events and remap. It owns only `BLU2USB_HID_SOURCE_SYNTHETIC_SHORTCUT` (new independent source), not any other Mouse or keyboard key.
- A bounded eight-action FIFO receives a shortcut on a **physical press edge**; only IDs 1 COPY and 7 ESCAPE are supported in this gate. Other saved IDs (including media/brightness/zoom) never falsely execute.
- A USB keyboard report must be accepted before advancing from keydown to keyup; keyup must be accepted before next queued action. The USB endpoint busy condition retries without collapsing the two reports or hanging the main loop.
- Disconnect, USB unmount/mount, BLE overflow, profile apply, Mouse removal, Pair New takeover and HAT lock cancel pending actions and release **only shortcut-owned keys**. Other HID sources remain intact. USB mount changes also invalidate cached last report to avoid confusing pre-unplug reports with acknowledgments from a new USB session.
- Existing VID/PID, keyboard 8-byte 6KRO, 2 USB HID interfaces, mouse pipeline, stored Custom/v3 profile, BLE bonds and Movement per Mouse stay unchanged. No Consumer descriptor and no CDC/debug adapter.
- Host test `mod05_button_chords_usb_backpressure` uses real C assertions in Release, simulates unsuccessful USB sends, cancellation, source ownership, preheld button, tap jitter, disabled mode and no premature wheel/move output. Source contract test checks main runtime wiring. Existing MOD-02/03/04 tests remain enabled.

## Gate acceptance checklist
See [planner MOD-05 physical checklist](https://github.com/remappingbridge/planner/blob/main/modifier-shortcuts/executions/MOD-05/physical-acceptance.md). In particular confirm **actual Linux app Escape** on clean tap and **actual clipboard Copy** without a context click and without extra Escape. The host test only verifies HID ownership/state, not the application's clipboard operation. Reboot/reconnect and check saved data. MOD-06 remains blocked until explicit operator acceptance.
