# Final HOPE architecture

This document describes the architecture that actually exists after HOPE-31. It intentionally does not invent a separate Core contract, device-registry service or coordinator layer that is not present in the program.

## Runtime composition

`src/app/main.c` is the product composition loop. It connects the concrete modules and translates runtime events into UX/model updates and application commands.

The main implementation layers are:

- `hat`: reads the Waveshare HAT controls;
- `interaction`: converts physical press/release state into release-triggered UI interactions and lock/unlock behavior;
- `ux_model`: owns the 29-screen navigation state, selection, saved-device presentation state and UI commands;
- `renderer`: projects the UX model to semantic 240x240 ST7789 cells/pixels;
- `ble_hogp`: owns BLE HID Mouse discovery, security, reconnect, HID report parsing, Pair New and saved-bond removal;
- `bt_runtime`: owns the Bluetooth runtime integration used by the Pico build;
- `profiles`: owns Passthrough, Standard, Escape and Custom profile state;
- `remap`: translates canonical Mouse input according to the selected profile;
- `hid_aggregator`: owns held HID state so profile changes/disconnects can release stale output safely;
- `logitech_hidpp`: contains the Logitech Lift-specific HID++ behavior;
- `storage`: persists product profile/custom state;
- `usb_hid`: exposes the stable USB HID product interfaces.

There is no user-facing Keyboard or Composite pairing flow in the final HOPE UX.

## UX authority

The runtime screen enum and templates live in `ux_model`. `BLU2USB_SCREEN_COUNT` is exactly 29.

Three HOME states are resolved from product state:

- connected saved Mouse -> `home-connected`;
- one or more saved Mice but none connected -> `home-searching` / retry flow;
- zero saved Mice -> automatic `searching-first`.

`searching-first` is not a HOME option.

## Saved Mouse identity

BTstack LE Device DB slot indexes are physical storage slots, not stable logical Mouse indexes. The firmware therefore enumerates real slots and maps them to logical saved-Mouse pages.

Saved Mouse names are persisted separately in the BTstack TLV product record and associated with BLE identity/IRK. This allows the name to remain visible while a Mouse is disconnected.

The connected saved Mouse is presented on page 1. Page ordering is presentation state; it does not change the persistent Mouse identity.

Removal resolves the selected logical Mouse to its BLE identity, removes equivalent bond records, removes its persisted name and disconnects first when the target is current.

## Remapping state

The final UI exposes four profile kinds:

- Passthrough;
- Standard;
- Escape;
- Custom.

Custom editing is performed in `EDIT CUSTOM REMAP` plus the five `WILL BECOME` screens. Applying Custom does not open a separate success screen: the accepted mapping is reflected in-place on `EDIT CUSTOM REMAP`.

## Event-driven screens

Some screens are entered by runtime events rather than by a HOME menu item:

- `SEARCHING FIRST MOUSE`: zero saved Mice;
- `FIRST MOUSE CONNECTED`: first-pair success;
- active profile feedback screens after a profile application;
- reconnect HOME transitions after BLE connected/disconnected/search events.

This distinction is why a screen can be canonical without being a menu option.

## Persistence boundaries

Bluetooth credentials remain in BTstack's LE Device DB. The persisted saved-name registry is a separate TLV record keyed by Mouse identity. Profile/custom product state is handled by the product storage/profile modules.

The final implementation does not create a second abstract device registry merely for documentation symmetry.
