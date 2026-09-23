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

static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    (void)blu2usb_ux_input(ux, control, false);
}

static void test_exact_layout(void)
{
    static const char *const expected[9] = {
        "NEW MOUSE NOT FOUND",
        "NO NEW MOUSE OUTSIDE",
        "THE LIST OF SAVED",
        "DEVICES WAS FOUND",
        "",
        "KEY A: RETRY NEW PAIR",
        "KEY B: BACK TRY SAVED",
        "KEY X: HELP",
        "KEY Y: LOCK",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

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
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[5][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_ux_option_count(&ux) == 0u);
}

static void test_retry_and_future_help_inert(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);

    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
}

static void test_back_uses_home_resolver(void)
{
    blu2usb_ux_model_t ux;

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);

    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);
}

static void test_lock_unlock_resolves_home(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, true);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, false);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
}

static void test_pair_help_returns_retry(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_PAIR_MOUSE;

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_PAIR_NEW);
    assert(ux.return_screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.screen == BLU2USB_SCREEN_RETRY_PAIR_NEW);
}

int main(void)
{
    test_exact_layout();
    test_retry_and_future_help_inert();
    test_back_uses_home_resolver();
    test_lock_unlock_resolves_home();
    test_pair_help_returns_retry();
    return 0;
}
