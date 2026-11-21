#include <assert.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t key) {
    const blu2usb_ux_command_t down = blu2usb_ux_input(ux, key, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    const blu2usb_ux_command_t up = blu2usb_ux_input(ux, key, false);
    assert(up.kind == BLU2USB_UX_COMMAND_NONE);
}

int main(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    assert(blu2usb_ux_option_count(&ux) == 4u);
    assert(strcmp(blu2usb_ux_screen_template(BLU2USB_SCREEN_HOME)->rows[2], " MOVEMENT OPTIONS") == 0);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(blu2usb_ux_option_count(&ux) == 4u);

    const blu2usb_screen_template_t *s = blu2usb_ux_screen_template(ux.screen);
    assert(strcmp(s->rows[0], "MOVEMENT OPTIONS") == 0);
    assert(strcmp(s->rows[1], " HORIZONTAL DPI 1") == 0);
    assert(strcmp(s->rows[2], " VERTICAL DPI 1") == 0);
    assert(strcmp(s->rows[3], " SCROLL INVERSE OFF") == 0);
    assert(strcmp(s->rows[4], " ROTATION 0-DEGREE") == 0);

    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(&ux, &frame);
    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);
    blu2usb_ui_project(&ux, &frame);
    assert(frame.cells[3][1].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(frame.cells[4][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    for (unsigned inverse = 0; inverse < 2; ++inverse) {
        const blu2usb_screen_id_t editor = inverse ?
            BLU2USB_SCREEN_SCROLL_INVERSE_ON : BLU2USB_SCREEN_SCROLL_INVERSE_OFF;
        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        assert(ux.screen == editor);
        tap(&ux, BLU2USB_CONTROL_KEY_X);
        assert(ux.screen == BLU2USB_SCREEN_SCROLL_HELP);
        tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(ux.screen == BLU2USB_SCREEN_SCROLL_HELP);
        tap(&ux, BLU2USB_CONTROL_KEY_B);
        assert(ux.screen == editor);
        tap(&ux, BLU2USB_CONTROL_KEY_B);
        assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS && ux.selection == 2u);
        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, true);
        const blu2usb_ux_command_t apply =
            blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, false);
        assert(apply.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(apply.movement.scroll_inverse == !inverse);
        assert(ux.screen == editor && ux.movement_current.scroll_inverse == inverse);
        blu2usb_ux_set_movement(&ux, apply.movement);
        ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
        ux.selection = 2u;
    }
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, true);
    const blu2usb_ux_command_t reset = blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, false);
    assert(reset.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(blu2usb_movement_valid(reset.movement));
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 0u);

    blu2usb_ux_set_mouse_connected(false);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    return 0;
}
