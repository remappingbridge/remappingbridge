# RemappingBridge

**Make your Bluetooth mouse work your way — on a pocket-sized device.**

No special RemappingBridge software to install on a compatible computer. Pair a BLE mouse with the **Raspberry Pi Pico 2 W (RP2350)**, choose button actions and pointer behavior using the little LCD, and send standard USB HID input to the host.

### Try it. Get it.

[**▶ OPEN THE LIVE SIMULATOR**](https://remappingbridge.github.io/) &nbsp; | &nbsp; [**⬇ DOWNLOAD FIRMWARE v3.2.4 (.UF2)**](https://github.com/remappingbridge/remappingbridge/releases/download/v3.2.4/remappingbridge-v3.2.4-pico2w.uf2)

[All releases / latest version](https://github.com/remappingbridge/remappingbridge/releases/latest) · [SHA-256 checksums for v3.2.4](https://github.com/remappingbridge/remappingbridge/releases/download/v3.2.4/SHA256SUMS.txt) · [Quick compatibility notes](#what-you-need-to-know-about-compatibility)

### Two small boards. Lots of possibilities.

<p align="center">
  <a href="https://www.raspberrypi.com/news/raspberry-pi-pico-2-w-on-sale-now/"><img src="https://www.raspberrypi.com/app/uploads/2024/11/Pico-2-W-1-1024x684.jpg" width="46%" alt="Official photo of the green Raspberry Pi Pico 2 W wireless microcontroller board, with the RP2350 chip and Micro-USB port." /></a>
  &nbsp;
  <a href="https://www.waveshare.com/Pico-LCD-1.3.htm"><img src="https://www.welectron.com/mediafiles/productimg/waveshare/devkit/LCD/Pico-LCD-1.3/Pico-LCD-1.3-details-1.jpg" width="42%" alt="Product photo of the blue Waveshare Pico-LCD-1.3 module, showing the square color LCD and five-way joystick." /></a>
</p>

<p align="center">
  <strong><a href="https://www.raspberrypi.com/news/raspberry-pi-pico-2-w-on-sale-now/">Raspberry Pi Pico 2 W (RP2350)</a></strong> &nbsp; · &nbsp;
  <strong><a href="https://www.waveshare.com/Pico-LCD-1.3.htm">Waveshare Pico-LCD-1.3 (SKU 19650)</a></strong>
</p>

<sub>Hardware photos are displayed from the Raspberry Pi and Welectron websites and linked to the corresponding manufacturer product information; they are not photographs of a finished RemappingBridge assembly. The RP2350 is the chip inside the Pico 2 W board. The Pico-LCD-1.3 is the Pico accessory with joystick and four buttons, <strong>not</strong> the 40-pin Raspberry Pi LCD HAT.</sub>

### What can you do?

- **Make a mouse button your shortcut key.** Use one Modifier button for **12 combinations** and choose from **30 keyboard and media actions**.
- **Make movement feel right.** Adjust horizontal and vertical speed separately, or rotate pointer movement by 90°, 180° or 270°.
- **Keep settings with your mouse.** Save individual profiles on the device, then use them with a compatible USB host without installing this project's desktop app.

**Curious?** [Explore the virtual HAT and LCD now — no hardware needed.](https://remappingbridge.github.io/) The website is a simulator, not a real Bluetooth pairing or firmware installer.

---

## Why use it?

Mouse customization is often tied to a particular computer or installed utility. RemappingBridge lets you configure supported mouse behavior on a small external device and use it with another USB HID-capable host without installing a dedicated RemappingBridge application there.

- **Everyday productivity:** assign actions such as Copy, Paste, Undo, Save, Search or media controls to supported mouse combinations instead of reaching for the keyboard each time.
- **Personalized input and accessibility:** experiment with button assignments and independent horizontal/vertical pointer gain to find a more comfortable control arrangement. These are customization options, **not a guarantee of accessibility for every person, mouse or application**.
- **Different workstations:** carry saved mouse-specific settings on the Pico rather than relying on this project's utility being installed on each computer. The host still has to support the USB HID usages and any keyboard shortcuts selected.

## How it works

### The signal path

![Data flow: a Bluetooth LE HID mouse connects wirelessly to RemappingBridge on the Pico 2 W, with an LCD and joystick for settings; the Pico sends USB HID mouse, keyboard and media-control signals to the host computer.](docs/assets/remappingbridge-flow.svg)

**BLE HID mouse → Pico 2 W + LCD controls → USB HID → computer.**

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

**Update precautions:** normal firmware updates use the UF2/BOOTSEL route without erasing the full flash. Supported migration from earlier global v2 profiles to v3 records is documented in the release history; saved BLE bonds and per-mouse Movement data are stored separately and should not be erased during normal supported upgrades. **Do not use `flash_nuke` as a routine update step.** Downgrading from v3 to v2 can lose or reset newer shortcut/profile preferences because v2 does not understand the v3 schema; record your mappings and consult the release documentation first.

## Documentation, support and license

- [Current v3.2.4 release notes](docs/releases/v3.2.4.md) and [v3.0.0 behavior/contract](docs/releases/v3.0.0.md).
- [Movement Options history](https://github.com/remappingbridge/planner/tree/main/movement-options), [Modifier Shortcuts planning](https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts) and [v3 maintenance decisions](https://github.com/remappingbridge/planner/tree/main/v3-maintenance).
- [Browser simulator source](https://github.com/remappingbridge/remappingbridge.github.io) and [firmware issues](https://github.com/remappingbridge/remappingbridge/issues).
- [Source code](src/), [tests](tests/) and [license](LICENSE).

**Public documentation language:** English. The firmware's source-controlled 9 × 21 screen text and semantic colors remain authoritative; documentation does not change those firmware strings. There is **no firmware change, new release or UF2 build** associated with this README revision.

### Development and diagnostics boundary

The production firmware intentionally does not add a debug USB CDC/UART interface, a debug screen or a separate diagnostic firmware identity. Troubleshooting uses source/host tests and the physical device UI. The web-only Inspector is not part of the Pico firmware.
