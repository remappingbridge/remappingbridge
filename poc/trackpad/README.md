# Magic Trackpad 2 on Pico 2 W

This experimental image connects one external Apple trackpad over Classic Bluetooth HID and presents a five-contact USB touchpad. The desktop interprets motion, taps, scrolling, pinching and swipes according to its gesture settings. Application-specific gestures depend on the application and desktop. The image forwards contact positions, identity, tip state and physical click. Pressure, Force Touch actions and host-controlled haptics are outside this implementation.

## Try the POC

1. Download the `magic-trackpad2-pico2w` artifact from a successful branch CI run. Unzip it. Hold BOOTSEL while plugging the Pico 2 W into USB, then copy `magic_trackpad2.uf2` to the mounted boot volume.
2. Disconnect the trackpad from its previous host, unplug its charging cable, then turn it off and on near the Pico. The image automatically scans for a matching inquiry name or Apple product ID. For a renamed device without advertised identity, configure its Bluetooth address at build time. Only put the intended trackpad in pairing mode during discovery.
3. Connect the Pico by a USB data cable to the test computer. Optional UART diagnostics (115200 baud, TX on GP0) distinguish discovery, HID connection, and the first Apple multitouch report. Touch the surface to trigger the latter.
4. On Windows 10/11 check the Touchpad settings and configure three- and four-finger actions. On Linux use `libinput list-devices` and `sudo libinput debug-events`. Check click and drag, two-finger scrolling (both axes), pinch and three-/four-finger swipes. Lift one finger mid-gesture, disconnect mid-drag, then reconnect and repeat. On ChromeOS Flex test overview and desk switching with its own touchpad settings. Record OS, trackpad model and connection diagnostics with each result.

## Build locally

Use the project's pinned Pico SDK 2.2.0 and ARM toolchain, with PICO_SDK_PATH set.

```sh
cmake -S . -B build-trackpad -DPICO_BOARD=pico2_w \
  -DBLU2USB_BUILD_PICO=ON -DBLU2USB_BUILD_TESTS=OFF -DBLU2USB_BUILD_TRACKPAD=ON
cmake --build build-trackpad --target magic_trackpad2 --parallel
```

The image is `build-trackpad/poc/trackpad/magic_trackpad2.uf2`. Add `-DTRACKPAD_ADDRESS=AA:BB:CC:DD:EE:FF` to the configure command to select a particular Classic Bluetooth address. The normal mouse/keyboard image is a separate target.

## Protocol references

- [Linux hid-magicmouse](https://github.com/torvalds/linux/blob/master/drivers/hid/hid-magicmouse.c) documents Bluetooth feature F1 with payload 02 01 and input 31 with nine-byte contact records.
- [mac-precision-touchpad](https://github.com/imbushuo/mac-precision-touchpad) demonstrates the Apple multitouch to Windows Precision Touchpad approach.
- [Microsoft report descriptors](https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchpad-sample-report-descriptors) define the host-facing contact and configuration collections.
- [BTstack HID host example](https://github.com/bluekitchen/btstack/blob/master/example/hid_host_demo.c) demonstrates Classic HID report mode.

Host tests exercise synthetic contact data. Physical pairing, desktop recognition and the gesture matrix require a Pico 2 W and trackpad.
