# MOD-01 — Remapping Options (physical test candidate)

Baseline: immutable tag `v2.0.0`. Gate branch: `feature/modifier-shortcuts-mod01`. Planning: https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts

## Implemented scope

- The Remapping Options menu contains **PASSTHROUGH DEFAULT**, **CUSTOM REMAPPING**, and **MODIFIER SHORTCUTS**.
- `JOY UP/DOWN` moves among these three options, `JOY PRESS` accesses supported routes, `KEY B` returns to Home, and `KEY X` opens Help with the updated names.
- Custom is in the second row. The third row is recognized, but pressing it is **intentionally a no-op until MOD-02**, which introduces COMBINATIONS 1 OF 6; no provisional screen is added.
- Only Passthrough and Custom appear in the HOME summary and Saved Devices PROFILE. A v2 setting with an active Standard/Escape profile can still be read, but is **normalized to PASSTHROUGH in RAM only** at startup. This gate neither updates the v2 schema nor changes bonds or writes flash during that transition. Persistent migration belongs to MOD-03.
- **Four former Standard/Escape confirmation screens** (APPLY and APPLIED) and their UX commands were removed from the source and screen inventory in MOD-01: **38 → 34 runtime screens**. The Custom WILL BECOME `ESCAPE` target and serialized legacy profile IDs remain temporarily available to protect v2→v3 migration; MOD-04 and MOD-03 respectively replace them.
- USB HID, BLE, Movement, Saved Devices colors, and the v2 persistence format remain unchanged in this gate.

## Operator physical test procedure

1. Update with the `.uf2` built by CI for this commit **without erasing flash or using `flash_nuke`**. Connect a saved mouse until `home-connected` appears.
2. With `NO REMAP PASSTHROUGH` selected in the first row, press and release `JOY PRESS`. Verify exactly three options in the specified order, without STANDARD/ESCAPE.
3. In the menu, `JOY UP` at the first option wraps to the third; `JOY DOWN` returns to the first. Verify white selection and cyan for the active Passthrough profile.
4. `JOY PRESS` on PASSTHROUGH DEFAULT opens its confirmation/already-applied state; `KEY B` returns to the three options.
5. Select CUSTOM REMAPPING and press `JOY PRESS`; verify the usual Custom editor. Without changing an assignment, `KEY B` returns to the three options. If Custom is active, its name appears cyan.
6. Select MODIFIER SHORTCUTS and press `JOY PRESS`; in MOD-01 the menu must **remain on the same screen and focused option**, without opening Standard/Escape or triggering shortcuts. COMBINATIONS is introduced in MOD-02.
7. With the third option focused, `KEY X` opens REMAPPING OPTIONS HELP describing Passthrough Default, Custom Remapping, and Modifier Shortcuts; any key returns to the same menu option in this historical version.
8. `KEY Y` locks the LCD (backlight off); a `JOY PRESS` unlocks **without activating** the selected option. Return Home with `KEY B`.
9. Open SAVED DEVICES and confirm PROFILE shows PASSTHROUGH or CUSTOM, never STANDARD/ESCAPE; only the connected mouse name is cyan. Previously configured H/V DPI and ROTATION values remain.
10. Move the pointer, scroll, and click; validate the configured rotation and H/V gains. Disconnect and reconnect the mouse, then revisit the menu. The saved mouse must retain its bond, name, and Movement.

## Not accepted or implemented in this gate

- COMBINATIONS and SHORTCUTS pages, shortcut assignments/persistence, the MODIFIER button, gestures, media actions, and HID descriptor changes.
- The `v2.0.0` tag and `main` are not modified by this historical candidate branch; the PR remains draft until acceptance.
