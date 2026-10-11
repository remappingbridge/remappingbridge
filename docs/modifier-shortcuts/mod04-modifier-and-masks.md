# MOD-04 — Custom MODIFIER, source exclusivity and reversible UNAVAILABLE

Baseline: operator-accepted MOD-03 commit `6e5ff857185466a459c1caa8633a1847f49ed230`. Firmware branch `feature/modifier-shortcuts-mod04`, draft PR #24. This candidate **does not execute any keyboard shortcuts**; the runtime executor starts at MOD-05.

## UI entry and layout
`HOME connected → REMAPPING OPTIONS → CUSTOM REMAPPING → {LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD} WILL BECOME`. Each WILL BECOME has precisely 6 rows in this order:

1. LEFT
2. RIGHT
3. MIDDLE
4. FORWARD
5. BACKWARD
6. MODIFIER

`KEY A: APPLY AND BACK` requests a draft write; only after verified product-flash commit does the UX return to EDIT CUSTOM. `JOY PRESS` on a WILL BECOME target is a no-op: **only KEY A applies**. `KEY B` cancels without writing. `KEY A: APPLY CUSTOM` applies the persisted Custom candidate, updates active mouse remap and derived modifier source after successful flash commit. There is no ESCAPE target in these menus; Escape remains shortcut catalog item #7.

## Corrected UI confirmation and colors

- After a committed target returns to EDIT CUSTOM REMAP, its row must show the complete value (e.g. `LEFT IS MODIFIER`), never an empty target name.
- When Custom is successfully applied with no pending draft, EDIT CUSTOM REMAP shows both `JOY PRESS: ACCESS` and `KEY B: BACK`; during a pending draft it shows `KEY A: APPLY CUSTOM`.
- On WILL BECOME screens, only the saved target is cyan when not selected; the selected option is white; all other options use the system's actionable gray. Cyan is recomputed from the current target on every frame, so switching MIDDLE from FORWARD to BACKWARD clears cyan from FORWARD.
- `JOY UP/DOWN` retains option selection; `JOY PRESS`, `KEY X`, `JOY LEFT/RIGHT` do not apply/save. `KEY Y` retains global lock/unlock to HOME.

## Applied vs draft, masks
- Modifier target ID 6 is explicitly distinct from legacy ESCAPE ID 5. Only one physical source among LEFT/RIGHT/MIDDLE/FORWARD/BACKWARD can be Modifier.
- Choosing a different modifier in the Custom draft automatically reassigns the previous Modifier source to its identity target. Nonmodifier remaps and global shortcut assignments are preserved.
- `COMBINATIONS` reflects the **applied** Custom template, not the uncommitted draft. A combo is UNAVAILABLE only if it uses the currently applied Modifier source (middle combo 1, left 2, right 3, forward 4, backward 5). Even when Passthrough is active, the stored Custom modifier remains visible as the unavailable source; Passthrough still sends original mouse clicks.
- `MODIFIER RELEASED`, WHEEL UP/DOWN and MOVE left/right/up/down are never unavailable.
- Masking does **not** delete any saved shortcut: its v3 ID stays stored and reappears when the Modifier source is changed. `JOY PRESS` and `KEY A RESET SELECTED` on an unavailable combo are no-ops, even if the masked ID has a saved value.
- Disabled `UNAVAILABLE` value uses the non-selectable STATIC tone (not cyan/white). Selected pair's label may be white. Other, unselected combination rows retain the accepted MOD-02 gray; the '+' bitmap renders.
- The only runtime treatment in this gate is consuming the physical button mapped to MODIFIER (the existing remap handler returns no host click or keyboard event). No MODIFIER RELEASED, chord, USB Consumer, wheel/move shortcut events until later gates.

## Migration/persistence
- v3 product profile: 26 bytes including 12 shortcut IDs, existing 2-slot CRC guarded flash writer. Exactly one new Modifier target permitted; duplicate Modifier v3 records rejected; unknown targets rejected without modifying runtime profile.
- v2 profile migration continues to replace retired ESCAPE targets with their physical identities. MOD-03's older v3 could contain legacy ESCAPE target=5; this gate similarly canonicalizes them to identity and verifies the rewrite, never interpreting ESCAPE as Modifier.
- Modifier/shortcuts are global; BLE bonds, per-device Movement and mouse names are untouched. Upgrade without `flash_nuke`; downgrade to v2 may reset global profile preferences.

## Physical acceptance scenarios from HOME connected
1. Flash MOD-04 production UF2 via BOOTSEL without erasing product/BT storage. Verify connected mouse, Saved Devices and Movement.
2. Navigate to Custom LEFT WILL BECOME; confirm 6 choices, MODIFIER last and ESCAPE absent. Visit all five WILL BECOME screens.
3. Select LEFT → MODIFIER and save draft; before applying Custom, no LEFT mask. Press KEY A APPLY CUSTOM; COMBINATIONS 2 OF 6 left becomes UNAVAILABLE.
4. On masked LEFT, JOY PRESS and KEY A RESET are inert; previous stored COPY/other action retained.
5. Select MIDDLE → MODIFIER and APPLY CUSTOM. LEFT reverts to LEFT, previous COPY reappears; MIDDLE becomes UNAVAILABLE and its stored shortcut remains hidden.
6. Repeat with RIGHT, FORWARD and BACKWARD: only matching button combo disabled, released/wheel/move options usable.
7. Apply Passthrough, confirm mouse normal click and stored modifier mask without executing shortcuts; return Custom.
8. Reboot and verify unique Modifier and latent assignments survive along with BLE bond, H/V DPI and rotation. Lock Y/first unlock event and Saved Devices cyan rule remain stable.

**Do not mark accepted before operator physically tests the exact CI UF2 and explicitly approves MOD-04.**
