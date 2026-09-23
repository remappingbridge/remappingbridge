#include <assert.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char out[BLU2USB_RENDERER_TEXT_COLS + 1u])
{
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        out[col] = frame->cells[row][col].character;
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    while (end > 0u && out[end - 1u] == ' ') --end;
    out[end] = '\0';
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    (void)blu2usb_ux_input(ux, control, false);
}

static void test_exact_layout(void)
{
    static const char *const expected[9] = {
        "DEVICE NOT FOUND HELP",
        "TO CONNECT A SAVED",
        "DEVICE FIRST UNPLUG",
        "CURRENTLY CONNECTED",
        "MOUSE AND PRESS THE",
        "KEY B TO BACK UNTIL",
        "SEARCHING APPEARS.",
        "",
        "ANY KEY: BACK",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW;

    project(&ux, &frame);

    assert(!frame.learn_background);
    assert(!frame.didactic_layout);
    assert(frame.hint_start_row == 8u);
    assert(blu2usb_renderer_background_rgb565(&frame, 0u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 5u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 6u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 7u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 8u) == BLU2USB_COLOR_DARK_MAGENTA);

    for (unsigned row = 0u; row < 9u; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
    }

    assert(frame.cells[0][0].tone == BLU2USB_UI_TONE_TITLE);
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[5][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[6][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[8][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_ux_option_count(&ux) == 0u);
}

static void test_x_opens_help(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

    tap(&ux, BLU2USB_CONTROL_KEY_X);

    assert(ux.screen == BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);
    assert(ux.return_screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
}

static void test_any_key_returns_retry_consumed_and_y_never_locks(void)
{
    for (unsigned raw = 0u; raw < BLU2USB_CONTROL_COUNT; ++raw) {
        blu2usb_ux_model_t ux;
        blu2usb_ux_init(&ux);
        blu2usb_ux_set_saved_device_count(&ux, 1u);
        ux.screen = BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW;
        ux.return_screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

        const blu2usb_control_t control = (blu2usb_control_t)raw;
        const blu2usb_ux_command_t press =
            blu2usb_ux_input(&ux, control, true);
        assert(press.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));

        const blu2usb_ux_command_t release =
            blu2usb_ux_input(&ux, control, false);
        assert(release.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);
        assert(ux.selection == 0u);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
    }
}

static void test_help_exit_does_not_start_pair_new(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);

    /* Only A from retry enters active Pair New. */
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
}

int main(void)
{
    test_exact_layout();
    test_x_opens_help();
    test_any_key_returns_retry_consumed_and_y_never_locks();
    test_help_exit_does_not_start_pair_new();
    return 0;
}
