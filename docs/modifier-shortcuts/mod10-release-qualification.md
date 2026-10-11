# MOD-10 — Release qualification candidate (NOT a published release)

**Base:** exact operator-approved MOD-09 SHA `dcc85dfc86ef140bf8acb2be54488f0ce07b3000`.
**Branch:** `feature/modifier-shortcuts-mod10`.
**Status:** candidate, requiring CI + Pico 2 W production UF2 + human physical acceptance.
**No tag/release/main merge authorized.**

## Scope
- Exercise the actual HOME CONNECTED → REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS → SHORTCUTS (five pages, 30 actions) → APPLY AND BACK flow, re-opening each of 30 actions in host tests.
- Require verified v3 serialization/reread before UI confirmation. KEY B cancellation and RESET SELECTED must not alter unrelated slots.
- Validate a masked MOD+LEFT assignment survives changing MODIFIER from LEFT to MIDDLE, is unmasked thereafter, persists after reboot, rejects CRC-corrupt data, and without a configured Modifier events cannot produce shortcuts.
- Exercise **all 30** semantic Keyboard/Consumer actions as separate down and up phases under simulated USB endpoint backpressure with minimum hold intervals, no stuck aggregator ownership. This proves typed HID contract only, NOT actual action in KDE/Android.
- Audit production source: stable 2 USB HID interfaces, report protocol and Boot keyboard compatibility, no CDC/debug firmware, preserved v3 migration, BLE identity-scoped removal, USB neutral Consumer release, lock/cancel, saved-device name-only cyan, Logitech/Movement no redesign.
- Run every inherited host CTest and Pico 2 W build from pinned toolchain; attach exact UF2 and sha256.
- Rehearse non-destructive upgrade and rollback/downgrade **physically**, with explicit warning: v2.0.0 cannot parse v3 shortcut config and can reset global mapping. Never call `flash_nuke` to install this candidate or downgrade. Make a manual record of all 12 slots and profile/Modifier before attempted downgrade; ensure BLE bonds are retained. Do not claim v3 preservation on v2 downgrade.

## Physical acceptance / release decision
The operator checks the 30 actions against declared host(s) and target apps, all 12 combination types, Movement/Logitech, startup/persistence/rollback, USB enumeration, BLE reconnect and stuck-key prevention. An action's HID usage being emitted is *not* proof that the OS or a specific application responds. Record host-specific exceptions, including the previously observed Obsidian zoom discrepancy, without guessing or hiding them.

Neither CI success nor the downloadable UF2 is operator acceptance or permission to publish. The gate's final result must be a numbered physical test log; release/publication requires a **separate** explicit request.
