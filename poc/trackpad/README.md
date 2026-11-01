# Magic Trackpad 2 on Pico 2 W

The POC builds two USB profiles for the same Classic Bluetooth receiver. Select the UF2 for the test host.

| Firmware | USB interface | Physical validation |
| --- | --- | --- |
| `magic_trackpad2_macos.uf2` | Experimental wired Magic Trackpad 2 emulation for macOS | Pending |
| `magic_trackpad2.uf2` | Precision Touchpad for Windows/Linux | Pending |

The macOS profile uses the wired MT2 identity, feature selection and activation handshake. It forwards the original contact records, including pressure and geometry, at a four-millisecond cadence. Its actuator interface accepts configuration, but does not forward haptic commands to the physical device. iPhone and Android behavior has not been validated.

## Test on a Mac

Hold BOOTSEL while connecting the Pico, then copy `magic_trackpad2_macos.uf2` to its boot volume. Disconnect the trackpad from the previous Bluetooth host, unplug its charging cable and turn it off and on. Keep the Pico on a USB data cable to the Mac. Move one finger, scroll a long Safari page with two fingers, then pinch a photo in Preview. Verify the relevant gestures are enabled in macOS Trackpad settings.

| Pico LED after touching the pad | Stage |
| --- | --- |
| Off | No active Bluetooth connection attempt or contact stream |
| Slow blink | Bluetooth HID connection in progress or waiting for Apple input |
| Fast blink | Valid Apple Bluetooth input received, waiting for a completed USB contact transfer |
| Solid | Mac enabled multitouch and the Pico completed a USB contact transfer |

To inspect USB enumeration even if Bluetooth pairing fails, run `system_profiler SPUSBDataType | grep -i -A 12 'Magic Trackpad'`. To inspect HID transport, run `hidutil list | grep -i trackpad` and look for the USB device with vendor/product 0x05ac/0x0265. Record the final LED state and both outputs if gestures fail.

## Test the Precision Touchpad profile

1. Download the `magic-trackpad2-pico2w` artifact from a successful branch CI run. Unzip it. Hold BOOTSEL while plugging the Pico 2 W into USB, then copy `magic_trackpad2.uf2` to the mounted boot volume.
2. Disconnect the trackpad from its previous host, unplug its charging cable, then turn it off and on near the Pico. The image automatically scans for a matching inquiry name or Apple product ID. For a renamed device without advertised identity, configure its Bluetooth address at build time. Only put the intended trackpad in pairing mode during discovery.
3. Connect the Pico by a USB data cable to the test computer. The onboard LED blinks while opening HID and turns solid after the first valid multitouch frame. A verified trackpad address is remembered across power cycles. Optional UART diagnostics (115200 baud, TX on GP0) distinguish discovery, HID connection, and the first Apple multitouch report. Touch the surface to trigger the latter.
4. On Windows 10/11 check the Touchpad settings and configure three- and four-finger actions. On Linux use `libinput list-devices` and `sudo libinput debug-events`. Check click and drag, two-finger scrolling (both axes), pinch and three-/four-finger swipes. Lift one finger mid-gesture, disconnect mid-drag, then reconnect and repeat. On ChromeOS Flex test overview and desk switching with its own touchpad settings. Record OS, trackpad model and connection diagnostics with each result.

## Build locally

Use the project's pinned Pico SDK 2.2.0 and ARM toolchain, with PICO_SDK_PATH set.

```sh
cmake -S . -B build-trackpad -DPICO_BOARD=pico2_w \
  -DBLU2USB_BUILD_PICO=ON -DBLU2USB_BUILD_TESTS=OFF -DBLU2USB_BUILD_TRACKPAD=ON
cmake --build build-trackpad --target magic_trackpad2 magic_trackpad2_macos --parallel
```

The image is `build-trackpad/poc/trackpad/magic_trackpad2.uf2`. Add `-DTRACKPAD_ADDRESS=AA:BB:CC:DD:EE:FF` to the configure command to select a particular Classic Bluetooth address. The normal mouse/keyboard image is a separate target.

## Protocol references

- [Linux hid-magicmouse](https://github.com/torvalds/linux/blob/master/drivers/hid/hid-magicmouse.c) documents Bluetooth feature F1 with payload 02 01 and input 31 with nine-byte contact records.
- [mac-precision-touchpad](https://github.com/imbushuo/mac-precision-touchpad) demonstrates the Apple multitouch to Windows Precision Touchpad approach.
- [Microsoft report descriptors](https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchpad-sample-report-descriptors) define the host-facing contact and configuration collections.
- [BTstack HID host example](https://github.com/bluekitchen/btstack/blob/master/example/hid_host_demo.c) demonstrates Classic HID report mode.

Host tests exercise synthetic contact data. Physical pairing, desktop recognition and the gesture matrix require a Pico 2 W and trackpad.

The Apple HID descriptor and feature data are device-capture values documented in [bt2usb](https://github.com/timothyb89/bt2usb/tree/53ecc1f728a44fdf20b7c950bd37c1855937b749/notes/mt2-translation). Its macOS tests informed the separate Apple profile, initialization sequence and regular USB frame timing.
