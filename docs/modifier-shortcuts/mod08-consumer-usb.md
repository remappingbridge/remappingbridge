# MOD-08 — HID Consumer Control on the existing Keyboard interface

**Source:** exact accepted MOD-07 commit `0b62137494ce34a86bbecb8d8ee42a7cdfa42385`, branch `feature/modifier-shortcuts-mod08`, PR #28 draft. Pending operator physical acceptance.

## USB descriptor (ADR-HID-01 option A)
- Interface **0** remains the legacy Mouse HID, with unmodified 5-byte report.
- Interface **1** remains HID Keyboard; **Report Protocol** now advertises two top-level collections: Keyboard (Report ID **1**, eight-byte 6KRO payload) and Consumer Control (Report ID **2**, two-byte little-endian HID Usage Page 0x0C payload). TinyUSB adds the report-ID byte automatically in Report Protocol, yielding 9-byte keyboard and 3-byte Consumer transfers. Both fit existing 16-byte endpoint buffers.
- **Boot Keyboard protocol** continues to send the historical eight-byte Keyboard payload without Report ID. Consumer reports are not emitted in Boot mode. Report-to-Boot transition and host enumeration are physical acceptance scenarios, not claimed from compiler validation alone.
- VID/PID, 2 HID interfaces, addresses, existing keyboard protocol class and USB configuration count are preserved; no third Consumer interface or CDC.

## Canonical Consumer usages
| Action ID | Semantic action | Consumer Usage (hex) |
|---|---|---|
| 19 | PLAY/PAUSE | 00CD |
| 20 | NEXT TRACK | 00B5 |
| 21 | PREVIOUS TRACK | 00B6 |
| 22 | VOLUME UP | 00E9 |
| 23 | VOLUME DOWN | 00EA |
| 24 | MUTE | 00E2 |
| 29 | BRIGHTNESS UP | 006F |
| 30 | BRIGHTNESS DOWN | 0070 |

The 22 Keyboard action IDs from MOD-07 are unchanged. The same 12 stable Modifier combination slots, screens and flash v3 schema are reused. All 30 IDs can now generate a typed HID report when the chosen protocol permits it. **Actual host action is still a separate proposition.** Media applications can ignore usages and Linux laptop displays may not respond to Consumer brightness even if the input device is recognized. There is no automatic translation to Linux system calls and no unsupported workaround under this gate.

## Action life cycle and ownership
1. A physical Modifier + button / wheel detent / directional MOVE gesture queues a semantic action; a clean solo release uses the same queue.
2. Domain maps the semantic ID to Keyboard or Consumer. Only one action runs at a time on the interface 1 USB endpoint. For Consumer a nonzero 16-bit usage is sent, then **USB acceptance** is required before sending zero, followed by acceptance of the zero report before processing the next queued action.
3. The 22 Keyboard actions retain their own source-scoped 6KRO/pressed-key guard, keydown ACK and keyup ACK; HID aggregator ownership is unaffected by Consumer actions.
4. On BLE disconnect/removal, USB report-protocol change, profile switching, lock or queue cancel, any accepted nonzero Consumer report is followed by an explicit zero report when the interface can send again. On physical USB dismount the old session state is invalidated, so no stale USB report is replayed into a new host session.
5. During Boot protocol Consumer action is cancelled without transmitting a malformed report. In Report Protocol each successful Consumer down/up uses Report ID 2; Keyboard uses Report ID 1.

## Validation and safety
- Host tests `mod08_consumer_8_usages_ack` and `mod08_combined_descriptor_boot_source` cover all eight IDs/usages, no cross-page confusion, Consumer busy, FIFO order mixed with Keyboard, owned release/cancel, exactly two interfaces, Report IDs, descriptor inclusion and production helper guards. Inherited MOD-07 22-key test and MOD-01..06 regressions run too.
- Pico 2 W build with pinned SDK 2.2.0/toolchain is the only production artifact to test. Do not erase flash or BLE pairings.
- [Full physical checklist](https://github.com/remappingbridge/planner/blob/main/modifier-shortcuts/executions/MOD-08/physical-acceptance.md) starts HOME connected and requires **actual** Linux media playback/volume/brightness change and USB enumeration/Boot 6KRO verification. Any brightness no-op must be reported rather than marked as supported. MOD-09 is blocked until operator approval.
