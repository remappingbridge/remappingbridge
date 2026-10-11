# MOD-06 — Modifier wheel and directional MOVE gestures

**Source lineage:** operator-accepted MOD-05 exact commit `ee11c4dedad7e1699912982d6469f3675cae2e88`, branch `feature/modifier-shortcuts-mod06`, draft PR #26. MOD-06 is a physical validation gate, not a release.

## Runtime behavior and scope

From HOME with paired BLE Mouse: REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS 4/5/6 OF 6. Wheel UP/DOWN have stable IDs 6/7 and MOVE LEFT/RIGHT/UP/DOWN are slots 8/9/10/11, independently editable through the existing 5-page SHORTCUTS picker. Configure a physical modifier through CUSTOM REMAPPING, apply Custom, then perform the wheel/motion gestures on the Mouse. No new screens, storage schema, bond/movement registry or USB interfaces.

### Wheel

- With Modifier held, each **signed canonical vertical wheel unit** produces one queued shortcut only if that slot has a currently supported HID action (MOD-05 executor: **COPY=1** or **ESCAPE=7**). Example: WHEEL UP → COPY, three positive units → three queued Ctrl+C chords, each with acked USB down/up. Event is consumed, so no accidental scroll at the host; Modifier release never produces an extra tap.
- Wheel DOWN uses opposite signed units and its own saved slot. A wheel message may have a horizontal pan component; the vertical assigned units are removed but horizontal pan is passed through unchanged.
- With no assignment or future-only unsupported shortcut (e.g. PLAY/PAUSE), vertical scroll still reaches host; it never masquerades as another shortcut. Any real wheel motion while Modifier is held suppresses `MODIFIER RELEASED`.
- FIFO remains bounded at eight entries; overflow drops additional shortcut triggers, never blocks BLE/HAT and never leaks captured scroll.

### MOVE

- Only while applied CUSTOM has exactly one Modifier and **at least one** directional MOVE slot is configured, pointer motion is captured from the first nonzero delta. The same event cannot feed either the existing HID aggregator or the Movement pipeline, so it **does not move the cursor** before/after recognition.
- Orientation is determined using the exact pure rotation helper shared with `blu2usb_movement_pipeline_push`: source dx/dy → rotate 0/90/180/270 degrees → classify before H/V gain. Rotation 90 in this firmware transforms positive source X to negative cursor Y (UP), 270 transforms it to DOWN. DPI multipliers cannot change gesture direction or activation thresholds.
- For initial physical evaluation, proposed tuning constants are **24 raw counts** on dominant axis, **2:1 axis dominance**, and **160 ms no MOVE samples** to rearm. These are experimental values for operator testing, **not claimed optimal**. Equal diagonal movement does not fire two shortcuts. Significant axis reversal before threshold resets accumulated candidate. While continuing to move after one gesture, only one action fires; only a new gesture after quiet can trigger another.
- MOVE LEFT (ID 8), RIGHT (9), UP (10), DOWN (11) trigger the configured action only if **COPY or ESCAPE** is assigned. Future-only actions may be saved but don't emit misleading HID events; with a MOVE slot configured, capture continues even if its saved semantic action awaits MOD-07/08.
- Minor sensor jitter below the MOD-05 eight-raw-count tap budget is ignored for the release shortcut; deliberate movement cancels the solo Modifier tap. With **no directional MOVE slots** configured, movement passes through unchanged with existing rotation, H/V gain and USB ACK/accumulation.
- PASSTHROUGH, Custom without a Modifier, disconnect and Mouse normal navigation continue their v2 behavior. Profile change, USB disconnect, BLE overflow, Lock, Mouse removal and Pair New handoff cancel gesture/shortcut state as in MOD-05.

### Explicit gate boundary

- Existing Keyboard HID allows **Escape** and **Copy** only at MOD-06. Catalog items such as Media/Brightness/Zoom are **not executed** until MOD-07/08. Do not mistake a saved text label for verified host action.
- Do not expand USB descriptor, add a USB interface, touch Bluetooth bonds, or merge/release in this gate.
- The host CI exercises 0/90/180/270 rotation, unequal H/V gains, wheel units/pan, diagonal/reversal, cursor capture, quiet rearm, jitter, disabled/no-assignment mode and queue bounds. **Physical calibration** is still required: the actual Mouse's wheel resolution and comfort with 24-count/160ms parameters may differ.

## Physical tests

Start HOME connected. Under Custom set LEFT as MODIFIER; assign WHEEL UP=ESCAPE and WHEEL DOWN=COPY in COMBINATIONS 4 OF 6; assign MOVE LEFT=COPY and MOVE RIGHT=ESCAPE in COMBINATIONS 5 OF 6; assign MOVE UP=COPY and MOVE DOWN=ESCAPE in COMBINATIONS 6 OF 6. Revisit through HOME if needed.

Hold LEFT: scroll up 1 and several units, scroll down, try pan; no vertical scroll spill or extra Modifier RELEASED event; actual ESCAPE / Copy must be recognized by Linux desktop. Hold LEFT: move left/right/up/down, equal diagonal, tiny jitter, rapid reversal, continuous direction, pause at least 160ms, move again. Verify one per intentional gesture and cursor stays still when a MOVE assignment exists. Change rotation 90 and 270 and H=2× V=1×; directions track **cursor coordinates**, not physical Mouse orientation, and gain doesn't affect threshold. Test with no MOVE assignments, without Modifier, in PASSTHROUGH and after reconnect, to verify normal motion.
