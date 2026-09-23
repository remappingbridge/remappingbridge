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

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux,
                                blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}

static void init_home(blu2usb_ux_model_t *ux, unsigned selection)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    blu2usb_ux_set_current_mouse_name(ux, "LOGITECH LIFT");
    ux->screen = BLU2USB_SCREEN_HOME;
    ux->selection = selection;
}

static void test_exact_layout_and_hint_region(void)
{
    static const char *const expected[9] = {
        "HOME CONNECTED HELP",
        "TO DISCONNECT THE",
        "CURRENTLY CONNECTED",
        "MOUSE, NAVIGATE TO:",
        "SAVED DEVICES >",
        "(MOUSE PAGE) > REMOVE",
        "DEVICE > REMOVE",
        "",
        "ANY KEY: BACK",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_HELP_HOME_CONNECTED;

    project(&ux, &frame);

    assert(!frame.learn_background);
    assert(!frame.didactic_layout);
    assert(frame.hint_start_row == 8u);
    assert(blu2usb_renderer_background_rgb565(&frame, 0u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 7u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 8u) ==
           BLU2USB_COLOR_DARK_MAGENTA);

    for (unsigned row = 0u; row < 9u; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
    }

    assert(frame.cells[0][0].tone == BLU2USB_UI_TONE_TITLE);
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[6][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[8][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_ux_option_count(&ux) == 0u);
}

static void test_x_opens_help_and_preserves_home_selection(void)
{
    blu2usb_ux_model_t ux;
    init_home(&ux, 3u);

    const blu2usb_ux_command_t open =
        tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(open.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);
    assert(ux.return_screen == BLU2USB_SCREEN_HOME);
    assert(ux.return_selection == 3u);
    assert(blu2usb_ux_option_count(&ux) == 0u);

    const blu2usb_ux_command_t back =
        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(back.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 3u);
}

static void test_every_hat_control_returns_consumed_and_y_never_locks(void)
{
    for (unsigned raw = 0u; raw < BLU2USB_CONTROL_COUNT; ++raw) {
        blu2usb_ux_model_t ux;
        init_home(&ux, 2u);
        tap(&ux, BLU2USB_CONTROL_KEY_X);
        assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);

        const blu2usb_control_t control = (blu2usb_control_t)raw;
        const blu2usb_ux_command_t press =
            blu2usb_ux_input(&ux, control, true);
        assert(press.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));

        const blu2usb_ux_command_t release =
            blu2usb_ux_input(&ux, control, false);
        assert(release.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_HOME);
        assert(ux.selection == 2u);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
    }
}

static void test_disconnect_retargets_help_return(void)
{
    blu2usb_ux_model_t ux;
    init_home(&ux, 1u);
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);

    /* Mirrors the app-side DISCONNECTED integration while Help stays visible. */
    blu2usb_ux_set_mouse_connected(false);
    ux.return_screen = BLU2USB_SCREEN_HOME_SEARCHING;
    ux.return_selection = 0u;

    const blu2usb_ux_command_t back =
        tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(back.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
    assert(ux.selection == 0u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
}

int main(void)
{
    test_exact_layout_and_hint_region();
    test_x_opens_help_and_preserves_home_selection();
    test_every_hat_control_returns_consumed_and_y_never_locks();
    test_disconnect_retargets_help_return();
    return 0;
}
