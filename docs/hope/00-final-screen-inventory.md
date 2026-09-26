# Final HOPE screen inventory

This document is the authoritative screen inventory for the completed HOPE UX.

The final inventory contains **29 canonical runtime screens**. The historical target of 30 was superseded when HOPE-23 was changed on 2026-09-26: the user-facing `learn-the-keys` feature was cancelled and removed from every HOME. HOPE-01 `searching-first` remains as the automatic zero-saved-Mouse bootstrap and is still canonical.

No extra screen is retained merely to preserve the historical count.

## Canonical inventory

| # | Runtime ID | Canonical screen | Gate | Entry / role |
|---:|---|---|---|---|
| 1 | `BLU2USB_SCREEN_SEARCHING_FIRST` | SEARCHING FIRST MOUSE | HOPE-01 | Automatic when zero saved Mice exist |
| 2 | `BLU2USB_SCREEN_MOUSE_SAVED` | FIRST MOUSE CONNECTED | HOPE-02 | First-pair success feedback |
| 3 | `BLU2USB_SCREEN_HOME` | home-connected | HOPE-03 | HOME while a saved Mouse is connected |
| 4 | `BLU2USB_SCREEN_SAVED_DEVICES` | saved-devices | HOPE-04 | Saved Mouse pages |
| 5 | `BLU2USB_SCREEN_REMOVE_THIS` | remove-this | HOPE-05 | Removal confirmation |
| 6 | `BLU2USB_SCREEN_PAIR_MOUSE` | pair-new | HOPE-06 | Pair a Mouse not already saved |
| 7 | `BLU2USB_SCREEN_RETRY_PAIR_NEW` | retry-pair-new | HOPE-07 | Pair New failure/retry |
| 8 | `BLU2USB_SCREEN_HOME_SEARCHING` | home-searching | HOPE-08 | Saved Mouse reconnect search |
| 9 | `BLU2USB_SCREEN_HOME_RETRY` | home-retry | HOPE-09 | Saved reconnect retry state |
| 10 | `BLU2USB_SCREEN_MOUSE_OPTIONS` | REMAPPING OPTIONS | HOPE-10 | Profile selector |
| 11 | `BLU2USB_SCREEN_PASSTHROUGH_APPLIED` | PASSTHROUGH ACTIVE | HOPE-11 | Active passthrough feedback |
| 12 | `BLU2USB_SCREEN_APPLY_DEFAULT` | APPLY STANDARD REMAP | HOPE-12 | Standard not-active/apply |
| 13 | `BLU2USB_SCREEN_DEFAULT_APPLIED` | STANDARD REMAP ACTIVE | HOPE-13 | Active standard feedback |
| 14 | `BLU2USB_SCREEN_APPLY_PASSTHROUGH` | APPLY PASSTHROUGH | HOPE-14 | Passthrough not-active/apply |
| 15 | `BLU2USB_SCREEN_APPLY_ESCAPE` | APPLY ESCAPE REMAP | HOPE-15 | Escape not-active/apply |
| 16 | `BLU2USB_SCREEN_ESCAPE_APPLIED` | ESCAPE APPLIED ACTIVE | HOPE-16 | Active Escape feedback |
| 17 | `BLU2USB_SCREEN_EDIT_CUSTOM` | EDIT CUSTOM REMAP | HOPE-17 | Custom editor and accepted Custom feedback |
| 18 | `BLU2USB_SCREEN_LEFT_WILL_BECOME` | LEFT WILL BECOME | HOPE-18 | Custom Left target |
| 19 | `BLU2USB_SCREEN_RIGHT_WILL_BECOME` | RIGHT WILL BECOME | HOPE-19 | Custom Right target |
| 20 | `BLU2USB_SCREEN_MIDDLE_WILL_BECOME` | MIDDLE WILL BECOME | HOPE-20 | Custom Middle target |
| 21 | `BLU2USB_SCREEN_FORWARD_WILL_BECOME` | FORWARD WILL BECOME | HOPE-21 | Custom Forward target |
| 22 | `BLU2USB_SCREEN_BACKWARD_WILL_BECOME` | BACKWARD WILL BECOME | HOPE-22 | Custom Backward target |
| 23 | `BLU2USB_SCREEN_HOME_SEARCHING_HELP` | HOME SEARCHING HELP | HOPE-24 | Help for home-searching |
| 24 | `BLU2USB_SCREEN_HOME_RETRY_HELP` | HOME RETRY HELP | HOPE-25 | Help for home-retry |
| 25 | `BLU2USB_SCREEN_HELP_PAIR_NEW` | PAIR NEW DEVICE HELP | HOPE-26 | Help for Pair New |
| 26 | `BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW` | MOUSE NOT FOUND HELP | HOPE-27 | Help for Pair New retry |
| 27 | `BLU2USB_SCREEN_HELP_HOME_CONNECTED` | REMOVE CONNECTED HELP | HOPE-28 | Help for connected HOME |
| 28 | `BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS` | REMAPPER OPTIONS HELP | HOPE-29 | Help for remapping options |
| 29 | `BLU2USB_SCREEN_HELP_REMOVE_THIS` | REMOVE MOUSE HELP | HOPE-30 | Help for remove-this |

HOPE-23 intentionally contributes **no screen**. Its accepted result is removal of the cancelled `LEARN THE KEYS` HOME option/route.

## Removed final-inventory residues

HOPE-31 removes the following pre-HOPE/G06 runtime screens rather than leaving them hidden:

- `MOUSE_STATUS`;
- `OTHER_DEVICES_STATUS`;
- `MOUSE_HELP`;
- `DEVICES_HELP`.

HOPE-31 also removes the unreachable duplicate `CUSTOM_APPLIED` screen. Custom application is confirmed in-place on `EDIT CUSTOM REMAP`, which is the accepted behavior.

The legacy internal name `BLU2USB_SCREEN_LEARN_KEYS` is removed. The automatic first-pair screen is canonically named `BLU2USB_SCREEN_SEARCHING_FIRST`.

## HOME option inventory

`home-connected` has exactly three selectable routes:

1. Remapping Options;
2. Saved Devices;
3. Pair New Mouse.

`home-searching` and `home-retry` each have exactly two selectable routes:

1. Saved Devices;
2. Pair New Mouse.

There is no user-selectable Learn The Keys route.
