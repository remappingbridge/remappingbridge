# RemappingBridge

**Use a Bluetooth mouse as a configurable USB mouse, with on-device button mapping, pointer movement and keyboard shortcuts.**

RemappingBridge is open-source firmware for the **Raspberry Pi Pico 2 W**. It connects to a compatible Bluetooth Low Energy (BLE) HID mouse, processes its input and presents **USB HID Mouse, Keyboard and Consumer Control** functions to a computer. The Pico handles the Bluetooth connection and settings; the computer does not need RemappingBridge-specific software.

[**Try the interactive browser simulator**](https://remappingbridge.github.io/) · [Latest firmware release](https://github.com/remappingbridge/remappingbridge/releases/latest) · [Technical documentation](docs/releases/v3.2.4.md)

![Data flow: a Bluetooth LE HID mouse connects wirelessly to RemappingBridge on the Pico 2 W, with an LCD and joystick for settings; the Pico sends USB HID mouse, keyboard and media-control signals to the host computer.](docs/assets/remappingbridge-flow.svg)

**In one line:** BLE HID mouse → Pico 2 W + LCD controls → USB HID → computer.

## Why use it?

Mouse customization is often tied to a particular computer or installed utility. RemappingBridge lets you configure supported mouse behavior on a small external device and use it with another USB HID-capable host without installing a dedicated RemappingBridge application there.

- **Everyday productivity:** assign actions such as Copy, Paste, Undo, Save, Search or media controls to supported mouse combinations instead of reaching for the keyboard each time.
- **Personalized input and accessibility:** experiment with button assignments and independent horizontal/vertical pointer gain to find a more comfortable control arrangement. These are customization options, **not a guarantee of accessibility for every person, mouse or application**.
- **Different workstations:** carry saved mouse-specific settings on the Pico rather than relying on this project's utility being installed on each computer. The host still has to support the USB HID usages and any keyboard shortcuts selected.

## How it works

1. **Bluetooth input:** a compatible BLE HID-over-GATT (HOGP) mouse pairs with the **Pico 2 W**, not with the computer's Bluetooth adapter.
2. **On-device configuration:** the firmware applies the connected mouse's saved profile, custom button behavior, movement scaling/rotation and optional Modifier Shortcuts. The attached LCD and joystick/buttons provide the user interface.
3. **USB output:** the Pico presents USB HID mouse, keyboard and consumer-control interfaces to the computer. RemappingBridge does not need a dedicated desktop driver or application on a host that supports these standard interfaces.

The USB cable carries HID input **to** the host. The firmware does not browse the host's files, read application state or automatically know which application is focused.

## Features in current firmware

- **Two mapping profiles — Passthrough and Custom.** Passthrough keeps ordinary mouse button behavior. Custom maps the five mouse buttons (Left, Right, Middle, Forward and Backward), including the option to designate **one** as a Modifier.
- **Modifier Shortcuts:** up to **12 combinations** involving Modifier release, other buttons, wheel scrolling and pointer movement; select among **30 keyboard/media actions**. The Modifier's own button combination is unavailable rather than treated as another ordinary click.
- **Movement Options:** independently set horizontal and vertical movement scaling from **0.25× to 3×** and rotate pointer motion by **0°, 90°, 180° or 270°**.
- **Saved Devices:** pair, reconnect, inspect and remove saved mice, with applied remapping, shortcuts and movement settings associated with individual mouse identities.
- **On-device interface:** a 240 × 240 LCD with a 9-line × 21-character layout, joystick and A/B/X/Y controls; Help, navigation, selection markers and LCD Lock.

### Examples to explore

**Assign Copy to a combination.** Configure one supported mouse button as **Modifier**, then choose a combination and the **COPY** action. Whether the shortcut performs Copy depends on the receiving application.

**Change pointer movement.** Set horizontal gain to **2×** and vertical gain to **0.75×**, or select a 90° rotation, without changing the host's mouse preferences through this project's software.

**Keep two mouse setups separate.** Save different Custom/Movement settings for different BLE mice, then reconnect the mouse whose settings you want to use.

These are descriptions of current firmware capabilities; they are not claims that every mouse, application or host has been tested.

## Try it in your browser

Open the **[RemappingBridge interactive simulator](https://remappingbridge.github.io/)** to explore the LCD and nine HAT controls before assembling hardware.

The single-page simulator includes **45 canonical firmware screens**, the three visual demo states (*Searching First Mouse*, *First Mouse Connected*, *Home Connected*), a **Virtual Mouse Lab** for local simulated pairing, saved-device profiles and failures, and an optional diagnostic Inspector. Its [browser test page](https://remappingbridge.github.io/tests/) uses separate temporary test data.

**Important:** the website is a simulation. It does **not** connect to a real Bluetooth mouse, control a physical Pico, send HID shortcuts to your computer or install firmware. Mouse names and profiles created in the main Virtual Mouse Lab are local browser data, not physical Pico records. GitHub README pages cannot embed the live JavaScript application; use the link above.

## What you need to know about compatibility

- **Input:** the firmware targets compatible **BLE HID-over-GATT mice**. A mouse that works only through a proprietary USB receiver or Bluetooth Classic is not automatically supported. Model-specific behavior may differ; Logitech Lift has targeted handling, not a general compatibility guarantee.
- **Controller:** this firmware is built for the **Raspberry Pi Pico 2 W**, with Bluetooth LE. A plain Pico 2 without wireless capability is not an equivalent substitute.
- **Computer:** use a host that accepts the Pico's **USB HID Mouse, Keyboard and Consumer Control** interfaces. You do not pair the mouse separately to the host for this data path.
- **Shortcuts:** a valid HID keyboard/media signal does not guarantee an action in every OS, browser, terminal or application. For example, **Zoom In** in v3.2.4 uses **Ctrl + Equal without Shift**, which works in the tested Obsidian/browser setup but may not trigger zoom in terminals expecting Ctrl + Shift + Equal. Brightness, Home and media-control behavior also depend on the host.
- **Boundaries:** this is a mouse-input bridge and remapper, **not a general Bluetooth keyboard pairing utility, USB gamepad emulator or universal accessibility solution**. **LCD Lock only locks the device's on-screen interface**; it does not stop ordinary BLE/USB mouse input.

Hardware identification, component illustrations and safe physical assembly are documented separately as that guide is finalized. Do not confuse the Pico display accessory with a 40-pin Raspberry Pi Linux LCD HAT.

## Current release and updates

**Current stable release: [v3.2.4](https://github.com/remappingbridge/remappingbridge/releases/tag/v3.2.4)** (published October 12, 2026). This release adjusts generic Zoom In for better compatibility with the tested Obsidian/browser setup; see the [v3.2.4 release notes](docs/releases/v3.2.4.md) for precise behavior and exceptions.

Use the **[latest GitHub release page](https://github.com/remappingbridge/remappingbridge/releases/latest)** for the current downloadable firmware and its checksum file. Direct UF2 filenames contain version numbers and must be checked against the current release; complete installation instructions will be maintained in the dedicated installation guide.

**Update precautions:** normal firmware updates use the UF2/BOOTSEL route without erasing the full flash. Saved BLE bonds and per-mouse settings should be preserved during normal supported upgrades. **Do not use `flash_nuke` as a routine update step.** Downgrading from v3 to v2 can lose or reset newer shortcut/profile preferences because v2 does not understand the v3 schema; record your mappings and consult the release documentation first.

## Documentation, support and license

- [Current v3.2.4 release notes](docs/releases/v3.2.4.md) and [v3.0.0 behavior/contract](docs/releases/v3.0.0.md).
- [Movement Options history](https://github.com/remappingbridge/planner/tree/main/movement-options), [Modifier Shortcuts planning](https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts) and [v3 maintenance decisions](https://github.com/remappingbridge/planner/tree/main/v3-maintenance).
- [Browser simulator source](https://github.com/remappingbridge/remappingbridge.github.io) and [firmware issues](https://github.com/remappingbridge/remappingbridge/issues).
- [Source code](src/), [tests](tests/) and [license](LICENSE).

**Public documentation language:** English. The firmware's source-controlled 9 × 21 screen text and semantic colors remain authoritative; documentation does not change those firmware strings. There is **no firmware change, new release or UF2 build** associated with this README revision.
