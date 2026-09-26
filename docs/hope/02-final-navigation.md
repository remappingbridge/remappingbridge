# Final HOPE navigation and control rules

## HOME resolver

Returning to HOME resolves dynamically:

- Mouse connected -> `home-connected`;
- saved Mouse exists but none is connected -> `home-searching`;
- no saved Mouse -> `searching-first`.

Unlock uses the same resolver.

## HOME routes

Connected HOME:

- Remapping Options;
- Saved Devices;
- Pair New Mouse.

Searching/Retry HOME:

- Saved Devices;
- Pair New Mouse.

The cancelled Learn The Keys option does not exist.

## Saved Devices

Saved Devices displays one logical Mouse per page. The connected Mouse is presented first. Disconnected saved names remain persisted.

Joy Left/Right changes page. Joy Press enters `remove-this`. `KEY B` returns through the HOME resolver.

`remove-this` pins the selected logical Mouse identity when the screen opens, so background reconnect/reordering cannot change the deletion target.

## Pair New

Pair New searches for a Mouse not already represented by the saved logical identities. Its retry and Help screens do not expose Keyboard or Composite pairing.

## Remapping

`REMAPPING OPTIONS` selects Passthrough, Standard, Escape or Custom.

Only the active profile is cyan when unselected. Applying a preset enters its active feedback screen. Custom remains in `EDIT CUSTOM REMAP` after commit, with the accepted mapping reflected by the current-state tones.

## Back

`KEY B` is contextual Back/Cancel except where the accepted UX explicitly consumes it:

- `searching-first`: didactic/inert;
- `first-mouse-connected`: inert except `KEY Y` Lock;
- `home-retry`: `KEY B` is intentionally not an action.

## Help

Every Help screen owns the HAT while visible. Any complete HAT interaction exits Help to its owner. `KEY Y` therefore means Back on Help and never Lock.

## Lock

`KEY Y` locks normal screens when the accepted screen policy allows it and at least one saved device exists. The first complete HAT interaction while locked is consumed only to unlock, then HOME is resolved from current connection/saved state.

`searching-first` is not a Lock screen.

## Removal

`KEY A` on `remove-this` starts one physical removal. Further confirmations are ignored while removal is pending.

If the target is connected, HID ownership is released and BLE disconnect occurs before bond/name deletion. If it is disconnected, another current Mouse is not disconnected.
