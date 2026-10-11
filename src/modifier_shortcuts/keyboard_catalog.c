#include "blu2usb/modifier_shortcuts/keyboard_catalog.h"

#include <stddef.h>

/* Standard HID Keyboard/Keypad usage page 0x07.
 * REDO, screenshots, REFRESH and zoom are Linux desktop/application
 * conventions, not firmware claims of OS-wide cross-platform actions.
 * Keypad +/- avoid relying on the US-layout Shift+= position. */
enum {
    KEY_A = 0x04u, KEY_C = 0x06u, KEY_F = 0x09u,
    KEY_R = 0x15u, KEY_S = 0x16u, KEY_V = 0x19u,
    KEY_X = 0x1bu, KEY_Z = 0x1du, KEY_0 = 0x27u,
    KEY_ENTER = 0x28u, KEY_ESCAPE = 0x29u,
    KEY_BACKSPACE = 0x2au, KEY_SPACE = 0x2cu,
    KEY_F2 = 0x3bu, KEY_F11 = 0x44u,
    KEY_PRINT_SCREEN = 0x46u, KEY_HOME = 0x4au,
    KEY_DELETE_FORWARD = 0x4cu,
    KEY_KP_MINUS = 0x56u, KEY_KP_PLUS = 0x57u
};
enum { MOD_CTRL = 0x01u, MOD_SHIFT = 0x02u };

static const blu2usb_keyboard_shortcut_t k_keyboard[31] = {
    [1] = {KEY_C, MOD_CTRL},          /* COPY */
    [2] = {KEY_X, MOD_CTRL},          /* CUT */
    [3] = {KEY_A, MOD_CTRL},          /* SELECT ALL */
    [4] = {KEY_V, MOD_CTRL},          /* PASTE */
    [5] = {KEY_Z, MOD_CTRL},          /* UNDO */
    [6] = {KEY_Z, MOD_CTRL | MOD_SHIFT}, /* REDO = Ctrl+Shift+Z */
    [7] = {KEY_ESCAPE, 0u},          /* ESCAPE */
    [8] = {KEY_SPACE, 0u},           /* SPACE */
    [9] = {KEY_ENTER, 0u},           /* ENTER / RETURN */
    [10] = {KEY_BACKSPACE, 0u},      /* BACKSPACE */
    [11] = {KEY_DELETE_FORWARD, 0u}, /* DELETE FORWARD, not Backspace */
    [12] = {KEY_F2, 0u},             /* RENAME in many file managers */
    [13] = {KEY_F11, 0u},            /* FULL SCREEN in browsers */
    [14] = {KEY_PRINT_SCREEN, 0u},  /* SCREENSHOT FULL */
    [15] = {KEY_PRINT_SCREEN, MOD_SHIFT}, /* SCREENSHOT AREA */
    [16] = {KEY_R, MOD_CTRL},        /* REFRESH */
    [17] = {KEY_F, MOD_CTRL},        /* FIND */
    [18] = {KEY_S, MOD_CTRL},        /* SAVE */
    /* 19..24: Consumer Media/Volume; intentionally NOT keyboard. */
    [25] = {KEY_HOME, 0u},          /* HOME PAGE, not Show Desktop */
    [26] = {KEY_KP_PLUS, MOD_CTRL}, /* ZOOM IN (browser/application) */
    [27] = {KEY_KP_MINUS, MOD_CTRL},/* ZOOM OUT */
    [28] = {KEY_0, MOD_CTRL},       /* RESET ZOOM */
    /* 29..30: Consumer Brightness; intentionally NOT keyboard. */
};

bool blu2usb_keyboard_shortcut_lookup(uint8_t action_id,
                                     blu2usb_keyboard_shortcut_t *out)
{
    if (out == NULL || action_id == 0u ||
        action_id > BLU2USB_KEYBOARD_SHORTCUT_MAX_ID ||
        k_keyboard[action_id].key_usage == 0u)
        return false;
    *out = k_keyboard[action_id];
    return true;
}
