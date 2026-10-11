# MOD-03 — SHORTCUTS editor and v3 persistence

**Candidate, physical acceptance pending** — derived from accepted MOD-02 corrective firmware on feature branch `feature/modifier-shortcuts-mod01` descending from immutable tag `v2.0.0`. This is not MOD-04 or HID execution.

## End-to-end navigation

`home-connected → REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS N OF 6 → JOY PRESS → SHORTCUTS X-Y OF 30 → KEY A: APPLY AND BACK → same COMBINATIONS page and slot`.

- The 12 stable combination IDs are 0..11, in the order documented in MOD-00.
- The 30 stable shortcut IDs are 1..30, with 0 meaning NOT CUSTOMIZED. Five pages, six entries each, title `SHORTCUTS 1-6 OF 30` etc., row 7 empty, row 8 `KEY A: APPLY AND BACK`. The values retain the source-approved spellings including `ENTER\RETURN`, `PLAY\PAUSE`, and `HOME SCREEN\PAGE`.
- `JOY UP/DOWN` cycles 6 choices, `JOY LEFT/RIGHT` cycles 5 pages; both wrap. Entering a previously assigned combination focuses its stored ID (page/row); without an assignment focus defaults to COPY. Paging back onto the current assigned item's page restores its line.
- `KEY A` in the picker requests persistence before returning. Failed persistence leaves picker open with the original assignment. `KEY B` cancels and returns to the origin combination page and option without changes. `JOY PRESS` does **not** save the selection.
- The selected shortcut line is white; previously persisted value may be cyan when not selected. On COMBINATIONS, both rows of the selected pair are white and unselected pair gray, including stored labels. The LCD '+' glyph from the accepted MOD-02 fix is preserved.
- `KEY A: RESET SELECTED` on a configured combination persists ID=0 and leaves same page/selection. With 0 already stored, reset is a no-op without flash write. `KEY Y` Lock remains global. No new unapproved Help screens.
- These are **configuration labels only**. No modifier can be chosen yet; there is no `UNAVAILABLE` masking until MOD-04. No keyboard chord, Consumer HID or media command is executed until MOD-05..08.

## Global storage migration

- Existing global profile v2 record is 14 bytes, checksum validated, before any transformation. New application-level v3 record is **26 bytes** (schema=3, active, draft flag, 5 Custom targets, 5 draft targets, 12 shortcut IDs, checksum), inside the existing 64-byte product payload. The existing two-slot CRC32 protected flash storage is reused; Bluetooth bonds, saved mouse names and Movement TLV are not written.
- For valid v2: Standard/Escape active profiles fall back to PASSTHROUGH; targets using legacy ESCAPE turn into the identity of the relevant source (also in the Custom draft), **not a MODIFIER**. All twelve shortcuts initialize to NONE. Candidate is written as v3 and verified through the existing dual-slot store. If the migration write fails, safe migrated values are usable in RAM, and migration can retry on a subsequent boot.
- For valid v3: restore profile, Custom/draft and 12 stored IDs 0..30; invalid ID, schema, checksum, truncated record or target => reject without partially mutating destination. Unknown app schema loaded with valid container CRC prohibits profile writes to avoid destroying recoverable data.
- Applying/resetting shortcuts uses a copy of the previous profile state, writes and verifies the new v3 record first, rolls back in RAM on failure and only **then** commits the new UI value and returns from the selector.
- Custom/WILL BECOME still supports the old ESCAPE target for compatibility inside the v3 model until MOD-04 replaces it with MODIFIER. Automatic migration never maps old ESCAPE to Modifier.
- **Downgrade caution:** original firmware v2.0.0 does not understand profile schema v3, so reverting could reset the global profile/shortcut preferences; BLE bonds and per-mouse Movement use different storage. Do not use `flash_nuke`.

## Operator physical test sequence

1. Flash new MOD-03 candidate `.uf2` via BOOTSEL without erasing flash; connect a previously saved Bluetooth Mouse.
2. HOME connected → Remapping Options → Modifier Shortcuts → Combinations 1 OF 6; JOY PRESS on `MODIFIER RELEASED IS` opens SHORTCUTS 1-6 OF 30, COPY focused.
3. Page JOY RIGHT through the 5 groups, checking 30 names in approved order, plus forward/back wrap. JOY UP/DOWN wraps within the 6 items.
4. Select COPY on the first combo; KEY A applies, returns to same combination/selection and displays COPY. Open again: COPY is already focused.
5. On `MOD + MIDDLE IS`, choose ESCAPE (page 2 first row) and KEY A; combo 1 retains COPY; combo 2 shows ESCAPE. KEY B after choosing another draft must cancel without changing the stored value.
6. Change `MODIFIER RELEASED IS` to PLAY\PAUSE via picker; confirm persisted action. Apply brightness-down to another combo to check page 5 and long action names.
7. On a configured combination, KEY A RESET SELECTED must show NOT CUSTOMIZED; resetting again does nothing. Adjacent assignments remain unchanged.
8. Reboot/unplug/replug Pico, reconnect previously saved mouse and revisit the same combinations. Previously configured shortcut labels must remain. Saved Devices still shows bonded mouse and correct DPI/rotation.
9. Check lock/unlock, back from picker preserving originating combination selection; navigate back to Home.
10. Verify normal mouse click, wheel, BLE reconnection and Movement while shortcuts are only configured, **not executed**. No unexpected shortcuts emitted by merely navigating.

**Gate boundary:** No implementation of Modifier source, combination unavailable masks, HID executor or new USB descriptor. Require CI PASS, production Pico UF2 plus SHA256, then operator physical acceptance before MOD-04.
