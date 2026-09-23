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

static void test_layout_and_selection(void)
{
    static const char *const expected[9] = {
        "DEVICE NOT FOUND",
        " SAVED DEVICES",
        " PAIR NEW MOUSE",
        " LEARN THE KEYS",
        "",
        "KEY A: RETRY SEARCH",
        "JOY UP / DOWN: SELECT",
        "JOY PRESS: ACCESS",
        "KEY X: HELP",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    project(&ux, &frame);

    assert(!frame.learn_background);
    assert(!frame.didactic_layout);
    assert(frame.hint_start_row == 5u);
    assert(blu2usb_renderer_background_rgb565(&frame, 0u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 4u) == BLU2USB_COLOR_BLACK);
    assert(blu2usb_renderer_background_rgb565(&frame, 5u) == BLU2USB_COLOR_DARK_MAGENTA);

    for (unsigned row = 0u; row < 9u; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
    }

    assert(frame.cells[0][0].tone == BLU2USB_UI_TONE_TITLE);
    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(blu2usb_ux_option_count(&ux) == 3u);

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
}

static void test_retry_controls(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_RETRY;

    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY_HELP);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);

    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
    assert(ux.selection == 0u);
}

static void test_destinations(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);

    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    ux.selection = 0u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);

    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);
}

static void test_lock_unlock_resolves_searching(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_RETRY;

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, true);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, false);

    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
    assert(ux.selection == 0u);
}

static void test_search_cancel_and_help_return_target_retry(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);

    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);

    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING_HELP;
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);
}

int main(void)
{
    test_layout_and_selection();
    test_retry_controls();
    test_destinations();
    test_lock_unlock_resolves_searching();
    test_search_cancel_and_help_return_target_retry();
    return 0;
}
