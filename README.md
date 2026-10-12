# RemappingBridge

Plug-and-play firmware for the **Raspberry Pi Pico 2 W**. It receives input from a Bluetooth HID mouse and exposes USB HID Mouse, Keyboard, and Consumer Control functionality to a host, without installing project-specific software on that computer.

**Language policy:** All public documentation, UI messages, accessibility text, test-facing messages, and GitHub Release descriptions in this repository and [the web simulator repository](https://github.com/remappingbridge/remappingbridge.github.io) must be written in **English**. This includes historical release notes and all new releases. Technical identifiers, release tags, literal firmware screen text, and checksums must not be translated.

## Latest stable firmware

**RemappingBridge 3.2.4** — MNT-15 was physically accepted. **ZOOM IN now sends Ctrl + Equal without Shift** for better compatibility with Obsidian and browsers on Linux. ZOOM OUT (Ctrl + Minus) and RESET ZOOM (Ctrl + 0) are unchanged. Some terminals require Ctrl + Shift + Equal for zoom and may not respond to this generic shortcut; terminal bindings vary. All MNT-13 menu focus markers, MNT-12 color rules, Bluetooth, USB HID, device-specific profiles, other shortcuts, and Movement are retained. [Release v3.2.4](https://github.com/remappingbridge/remappingbridge/releases/tag/v3.2.4).

### Modifier Shortcuts

- Two visible profiles: **PASSTHROUGH** and **CUSTOM**. The former STANDARD REMAP and ESCAPE REMAP options were removed from the user interface.
- One of five physical mouse buttons (Left, Right, Middle, Forward, or Backward) can be configured as a **MODIFIER**; it no longer produces its normal click while used for a combination.
- **12 combinations**: MODIFIER RELEASED; the five buttons; WHEEL UP/DOWN; and MOVE LEFT/RIGHT/UP/DOWN.
- **30 Keyboard/Consumer actions**, configured with `HOME CONNECTED → REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS 1..6 → SHORTCUTS 1..5 → KEY A: APPLY AND BACK`. A selected combination can be reset.
- Persisted profiles and shortcut assignments are associated **with individual Bluetooth mouse identities** (v3 records), including migration from the former global profile. `UNAVAILABLE` masks combinations that require the Modifier's own button without erasing their stored assignments.
- Protection against stuck keys, BLE/USB interruption, backpressure, Lock, and remapping transitions.

### Movement features retained from v2

- **Movement Options:** independent horizontal and vertical gains, 0°/90°/180°/270° rotation, and settings saved per paired Bluetooth mouse.
- BLE pairing and reconnection, Saved Devices, Logitech Lift HID++, Help/Lock, a 9×21-character LCD, wheel/pan, and stable USB identity.
- On **Saved Devices**, the title uses the standard magenta, only the connected mouse name appears cyan, movement/profile details remain yellow, `REMOVE DEVICE` is white when actionable, and hints are gray.

**Download and checksums:** [latest release](https://github.com/remappingbridge/remappingbridge/releases/latest) · [v3.2.3 release](https://github.com/remappingbridge/remappingbridge/releases/tag/v3.2.3) · [production UF2 for v3.2.3](https://github.com/remappingbridge/remappingbridge/releases/download/v3.2.3/remappingbridge-v3.2.3-pico2w.uf2) · [SHA256SUMS.txt](https://github.com/remappingbridge/remappingbridge/releases/download/v3.2.3/SHA256SUMS.txt). The direct filename is version-specific and must be updated for new releases.

Release documentation: [v3.2.3](docs/releases/v3.2.3.md), [v3.2.2](docs/releases/v3.2.2.md), [v3.0.3](docs/releases/v3.0.3.md), [v3.0.2](docs/releases/v3.0.2.md), [v3.0.1](docs/releases/v3.0.1.md), and [v3.0.0](docs/releases/v3.0.0.md).

## Browser simulator

The [interactive web simulator](https://remappingbridge.github.io/) is being developed as a static, single-page application based on the accepted firmware and [mouse-ui](https://github.com/remappingbridge/mouse-ui). The current WEB-01 baseline is a virtual LCD/HAT **preview**, not complete firmware emulation. It does not connect to physical Bluetooth/USB hardware. The GitHub README cannot execute embedded JavaScript; open the linked web app in your browser.

## Safe firmware updates

Use the normal **UF2 update in BOOTSEL mode**, without erasing the entire flash. The firmware migrates the legacy global v2 profile into v3 records. BLE bonds and per-mouse Movement data occupy other records and should not be erased during normal updates.

Downgrading from v3 to v2.0.0 is **not fully compatible**: v2 cannot read the v3 shortcut schema and may restore global-profile defaults. Record your 12 assignments and profile before downgrading. Never use `flash_nuke` for a routine update.

Some shortcut effects depend on the host OS and application. **Obsidian/KDE zoom**, Android HOME shortcuts, and actual host-screen brightness must be tested on the intended host. A valid HID usage does not guarantee the receiving application will implement it.

## Normative references

1. [Modifier Shortcuts — gates MOD-00..MOD-10](https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts): accepted v3 contract.
2. [v3.x maintenance gates](https://github.com/remappingbridge/planner/tree/main/v3-maintenance) and [v3.0.0 contract](docs/releases/v3.0.0.md): accepted MNT changes, navigation and colors.
3. [Movement Options](https://github.com/remappingbridge/planner/tree/main/movement-options): v2 functionality preserved in v3.
4. [v2.0.0 release notes](docs/releases/v2.0.0.md) and historical screen references, unless superseded.

In any UI conflict, **do not change frozen screen text or behavior unilaterally**. Preserve the 9×21 layout and the design-system colors.

## Debugging policy

No embedded debug tooling: do not add USB CDC/UART diagnostic interfaces, a debug screen, or a separate debug firmware build. Diagnostics use host/CI tests, the observable device UI, and host OS behavior.
