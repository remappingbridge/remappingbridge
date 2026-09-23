#include <assert.h>
#include <stdbool.h>
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

static void test_exact_pair_new_layout(void)
{
    static const char *const expected[9] = {
        "PAIR NEW MOUSE",
        "TRYING TO CONNECT",
        "A NEW MOUSE THAT",
        "IS NOT LISTED",
        "IN SAVED DEVICES",
        "",
        "KEY B: CANCEL",
        "KEY X: HELP",
        "KEY Y: LOCK",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;

    project(&ux, &frame);

    assert(!frame.learn_background);
    assert(!frame.didactic_layout);
    assert(frame.hint_start_row == 6u);
    assert(blu2usb_renderer_background_rgb565(&frame, 0u) ==
           BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 5u) ==
           BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 6u) ==
           BLU2USB_COLOR_DARK_MAGENTA);

    for (unsigned row = 0u; row < 9u; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
    }

    assert(frame.cells[0][0].tone == BLU2USB_UI_TONE_TITLE);
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[6][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_ux_option_count(&ux) == 0u);
}

static void test_pair_new_controls(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_X, true);
    project(&ux, &frame);
    assert(frame.cells[7][0].tone == BLU2USB_UI_TONE_EMPHASIZED);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_X, false);
    assert(ux.screen == BLU2USB_SCREEN_HELP_PAIR_NEW);
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);

    /* fresh model for cancel */
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    blu2usb_ux_set_mouse_connected(true);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);

    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);
}

static void test_pair_new_entry_paths(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_command_t cmd;

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 0u;
    cmd = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
    assert(cmd.kind == BLU2USB_UX_COMMAND_PAIR_MOUSE);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;
    ux.selection = 1u;
    cmd = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);

    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    ux.selection = 1u;
    cmd = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);
}

static void test_legacy_pair_copy_absent(void)
{
    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_PAIR_MOUSE);
    assert(screen != NULL);

    for (unsigned row = 0u; row < 9u; ++row) {
        const char *text = screen->rows[row] == NULL ? "" : screen->rows[row];
        assert(strstr(text, "SEARCHING BLE HID") == NULL);
        assert(strstr(text, "TARGET MOUSE") == NULL);
        assert(strstr(text, "AUTO SEARCH ACTIVE") == NULL);
        assert(strstr(text, "RETRY ON ERROR") == NULL);
    }
}

int main(void)
{
    test_exact_pair_new_layout();
    test_pair_new_controls();
    test_pair_new_entry_paths();
    test_legacy_pair_copy_absent();
    return 0;
}
