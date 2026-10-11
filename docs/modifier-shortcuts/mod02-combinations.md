# MOD-02 — Six COMBINATIONS pages (candidate for physical tests)

Baseline: MOD-01 physically accepted, branch `feature/modifier-shortcuts-mod01`, forked from stable tag `v2.0.0`. This gate adds **6** screens without implementing shortcut assignments or changing persisted data.

## Implemented behavior

- `home-connected → REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS 1 OF 6`.
- Six sequential pages, two combination slots per page, IDs 0..11:
  1. MODIFIER RELEASED IS / MOD + MIDDLE IS
  2. MOD + LEFT CLICK IS / MOD + RIGHT CLICK IS
  3. MOD + FORWARD IS / MOD + BACKWARD IS
  4. MOD + WHEEL UP IS / MOD + WHEEL DOWN IS
  5. MOD + MOVE LEFT IS / MOD + MOVE RIGHT IS
  6. MOD + MOVE UP IS / MOD + MOVE DOWN IS
- Each selected slot consists of **two body rows** (label and current value). Both rows become white when selected; unselected rows remain the standard off-white/yellow. A slot starts as `NOT CUSTOMIZED`. Header is magenta; the 9×21 physical layout is retained. No assignment is stored yet; no cyan highlight denotes an applied shortcut.
- `JOY UP/DOWN` changes selected slot with wrap. `JOY LEFT/RIGHT` changes pages with wrap 1 ↔ 6 **without losing logical selection**. `KEY B` returns to Remapping Options and preserves focus on MODIFIER SHORTCUTS; re-entry starts at combination 1 and first slot.
- `KEY A: RESET SELECTED` is an idempotent no-op for a NONE slot; no flash write. `JOY PRESS: ACCESS` is also a no-op in this gate, since MOD-03 will supply the 5 shortcut-picker pages and transactional persistence.
- `KEY X` is ignored (no new Help layout authorized). `KEY Y` keeps system Lock, and the first unlock event is consumed.
- No Modifier source can yet be chosen: Custom WILL BECOME retains `ESCAPE` until MOD-04. In this gate no slot is `UNAVAILABLE`; all are `NOT CUSTOMIZED`.
- Bluetooth pairing/bonds, mouse movement/rotation, USB HID identity, schema-v2 profile serialization and Saved Devices remain unchanged.

## Physical test sequence

1. Flash the **candidate UF2 from MOD-02 CI** without flash erase / `flash_nuke`, connect the previously bonded mouse.
2. From HOME connected, press `JOY PRESS` (Remapping Options), select `MODIFIER SHORTCUTS` (third item) with two `JOY DOWN` presses and `JOY PRESS`. Confirm `COMBINATIONS 1 OF 6`, body labels and `NOT CUSTOMIZED`.
3. Press `JOY RIGHT` five times; confirm pages 2,3,4,5,6 with labels in the order above. Press `JOY RIGHT` again: page 1. `JOY LEFT` from page 1: page 6.
4. On page 6, `JOY DOWN`: select **both** rows of the second slot. `JOY RIGHT` wraps to page 1 with the second slot still selected. `JOY UP` selects the first slot, `JOY UP` again wraps to the second.
5. `KEY A` on either slot does **not** alter `NOT CUSTOMIZED` or the page; `JOY PRESS` likewise remains on the page. Neither event emits keyboard shortcuts.
6. `KEY X` has no action; `KEY Y` locks. First `JOY PRESS` after locking only unlocks and returns HOME.
7. Reopen COMBINATIONS, `KEY B` back to Remapping Options with MODIFIER SHORTCUTS selected; `KEY B` returns HOME.
8. Check preserved existing mouse clicking, scrolling, rotation, H/V Movement values and Saved Devices connected-name-only cyan policy. Disconnect/reconnect the mouse and repeat HOME → COMBINATIONS.

## Validation

Automated CI includes `mod02_combinations_physical_flow`, `mod02_combinations_source_contract`, inherited regressions, runtime inventory **40 screens** (34 from MOD-01 + 6). Gate not accepted until operator's physical confirmation. No merge or release.
