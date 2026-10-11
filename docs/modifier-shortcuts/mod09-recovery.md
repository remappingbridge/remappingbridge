# MOD-09 — Recovery, queue safety, Saved Devices and host integration

**Base:** physically accepted MOD-08 commit `5bfde2c4dc21aef3505ea9884a4878ae8ee8ec6f`.
**Branch:** `feature/modifier-shortcuts-mod09`. **Status:** candidate; operator physical acceptance pending.
**Scope:** no new semantic actions, USB interfaces, layouts, Movement gains or Logitech vendor protocol. No release/merge.

## Runtime hardening
1. `discard_stale_mouse_motion` flushes the pending movement accumulator and relative/wheel/pan from the canonical aggregator whenever USB mounted-state transitions, BLE disconnects, pair-new promotes another mouse or the BLE runtime signals overflow. This prevents motion produced in a lost transport session from being replayed as a jump or a scroll in the new session. Source-owned held Mouse/Keyboard buttons remain managed independently.
2. An accepted shortcut configuration write now cancels old pending/running shortcut actions *before* installing the new global action array. Old synthetic key/drag owners are released. A previously accepted Consumer usage is explicitly neutralized by the existing USB release-barrier on the next report-capable tick. Failed flash persistence leaves the original runtime mapping unchanged.
3. The inherited MOD-06 future-ID contract is unchanged: a nonzero MOVE assignment captures the gesture, but unsupported action IDs never generate USB key or Consumer reports. No speculative changes to pointer semantics under Modifier.
4. Existing safeguards remain the sole implementation: bounded eight-action FIFO; endpoint ACK before release; non-blocking Consumer 60 ms and Keyboard 25 ms hold; disconnect/lock/profile/removal cancellations; HID source-isolated refs; saved active identity highlighting. No parallel USB event producer is introduced.

## Contract regression tests
- `mod09_recovery_cancel_overflow_and_ownership`: cancel Consumer while USB busy, idempotent lock/disconnect cancel, synthetic Ctrl+C key-down canceled after USB ACK, unrelated physical keyboard key preserved, bounded wheel burst and unsupported MOVE IDs.
- `mod09_app_saved_devices_safety_source`: production path for motion invalidation, config mutation cancellation, Consumer neutralization, active Saved Devices name-only cyan, failure handling in BLE and source eligibility.
- All inherited MOD-01..08 tests must pass as well.
- Pico 2 W UF2 built via pinned GitHub CI. No diagnostic USB CDC or debug UF2.

## Manual acceptance starts at HOME CONNECTED
1. Enter HOME → REMAPPING OPTIONS → MODIFIER SHORTCUTS. Confirm existing 12 combination assignments and 30 action labels are unchanged after flash upgrade, and reset still targets only the selected slot.
2. Configure Left as MODIFIER and Right as COPY; trigger and release; verify no Left click or permanently held Ctrl. Repeat while normal left/right drag is active in another profile.
3. Start a Keyboard shortcut, disconnect the mouse while it is active; keyboard returns to neutral and existing physical keyboard input still functions.
4. Start PLAY/PAUSE or VOLUME UP Consumer action; disconnect mouse/USB/lock with Modifier still held; after reconnection, the next action works once, with no stuck volume or repeat.
5. With wheel configured to VOLUME UP, spin several detents under Modifier and disconnect/lock while actions are queued; HAT stays responsive, no stale batch executes on reconnect.
6. Configure MOVE LEFT and other movement gestures; unplug/replug USB during an active gesture. After reconnection cursor does not jump from old queued deltas, and normal movement resumes.
7. Trigger SCREENSHOT AREA, begin second-button selection drag and disconnect/lock/USB reset. The synthetic Left click must be released; no ghost drag after recovery.
8. Switch profiles while Modifier/secondary button held; no stale queued action from previous profile; normal Custom/Passthrough behavior resumes after fresh press.
9. Edit an assigned shortcut while an older action is pending: after APPLY AND BACK, the old action must not execute and the new action must be reflected on next fresh trigger. An aborted edit by KEY B changes nothing.
10. Open SAVED DEVICES with multiple saved mice: only the currently connected mouse **name** is cyan; page counter, PROFILE, DPI, ROTATION and all hints remain system yellow. Disconnect and reconnect: no inactive name turns cyan.
11. Remove an inactive saved mouse, then the active saved mouse. Remaining entries, HOME and Movement settings follow the saved device/identity; no stale shortcut, no wiped unrelated settings.
12. Power-cycle, reconnect the saved mouse and verify Movement H/V gain/rotation, custom modifier and 12 shortcuts remain; Logitech Lift Forward hold/drag, normal wheel/pan and Help/lock still work.
13. Repeat with intentionally busy USB endpoint (e.g. repeated media/keyboard actions while app and OS are occupied); never force USB re-enumeration or produce a stuck button.
14. No flash erase, flash_nuke, Bluetooth bond reset, main merge, release or MOD-10 without explicit operator acceptance.

## Qualification status
The host regression suite and production build do not constitute physical acceptance. Log discrepancies separately for HID emitted, Linux input event received and actual application/system effect. The previously observed Obsidian zoom discrepancy remains intentionally outside this recovery gate; no app-specific shortcut mapping was introduced.
