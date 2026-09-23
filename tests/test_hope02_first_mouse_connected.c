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

static void assert_span(const blu2usb_ui_frame_t *frame,
                        unsigned row, unsigned col, unsigned len,
                        blu2usb_ui_tone_t tone)
{
    for (unsigned i = 0u; i < len; ++i)
        assert(frame->cells[row][col + i].tone == tone);
}

static void assert_row_tone(const blu2usb_ui_frame_t *frame,
                            unsigned row, blu2usb_ui_tone_t tone)
{
    bool visible = false;
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
        if (frame->cells[row][col].character == ' ') continue;
        visible = true;
        assert(frame->cells[row][col].tone == tone);
    }
    assert(visible);
}

static void test_layout_background_and_geometry(void)
{
    static const char *const expected[BLU2USB_RENDERER_TEXT_ROWS] = {
        "FIRST MOUSE CONNECTED",
        "       JOY UP",
        "  JOY    JOY    JOY",
        "  LEFT  PRESS  RIGHT",
        "      JOY DOWN",
        " KEY A         KEY X",
        " KEY B         KEY Y",
        "",
        " KEY Y: LOCK",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_SAVED;
    project(&ux, &frame);

    assert(!frame.learn_background);
    assert(frame.didactic_layout);
    assert(frame.hint_start_row == 8u);
    assert(blu2usb_renderer_separator_boundary_y(&frame) == 203u);

    for (unsigned row = 0u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
        const uint16_t expected_bg =
            row < 8u ? BLU2USB_COLOR_BLACK : BLU2USB_COLOR_DARK_MAGENTA;
        assert(blu2usb_renderer_background_rgb565(&frame, (uint8_t)row) == expected_bg);
    }

    assert_row_tone(&frame, 0u, BLU2USB_UI_TONE_TITLE);
    for (unsigned row = 1u; row <= 6u; ++row)
        assert_row_tone(&frame, row, BLU2USB_UI_TONE_ACTIONABLE);
    assert_row_tone(&frame, 8u, BLU2USB_UI_TONE_ACTIONABLE);

    assert(blu2usb_renderer_text_y(&frame, 0u) == 8u);
    assert(blu2usb_renderer_text_y(&frame, 1u) == 39u);
    assert(blu2usb_renderer_text_y(&frame, 2u) == 64u);
    assert(blu2usb_renderer_text_y(&frame, 8u) == 214u);
}

static void press_and_project(blu2usb_ux_model_t *ux,
                              blu2usb_control_t control,
                              blu2usb_ui_frame_t *frame)
{
    (void)blu2usb_ux_input(ux, control, true);
    assert(ux->screen == BLU2USB_SCREEN_MOUSE_SAVED);
    project(ux, frame);
}

static void release_inert(blu2usb_ux_model_t *ux, blu2usb_control_t control)
{
    const blu2usb_ux_command_t cmd = blu2usb_ux_input(ux, control, false);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux->screen == BLU2USB_SCREEN_MOUSE_SAVED);
    assert(!blu2usb_interaction_is_locked(&ux->interaction));
}

static void test_didactic_feedback_and_inert_controls(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t held;

    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_SAVED;

    press_and_project(&ux, BLU2USB_CONTROL_JOY_UP, &held);
    assert_span(&held,1u,7u,6u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_JOY_UP);

    press_and_project(&ux, BLU2USB_CONTROL_JOY_LEFT, &held);
    assert_span(&held,2u,2u,3u,BLU2USB_UI_TONE_EMPHASIZED);
    assert_span(&held,3u,2u,4u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_JOY_LEFT);

    press_and_project(&ux, BLU2USB_CONTROL_JOY_PRESS, &held);
    assert_span(&held,2u,9u,3u,BLU2USB_UI_TONE_EMPHASIZED);
    assert_span(&held,3u,8u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_JOY_PRESS);

    press_and_project(&ux, BLU2USB_CONTROL_JOY_RIGHT, &held);
    assert_span(&held,2u,16u,3u,BLU2USB_UI_TONE_EMPHASIZED);
    assert_span(&held,3u,15u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_JOY_RIGHT);

    press_and_project(&ux, BLU2USB_CONTROL_JOY_DOWN, &held);
    assert_span(&held,4u,6u,8u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_JOY_DOWN);

    press_and_project(&ux, BLU2USB_CONTROL_KEY_A, &held);
    assert_span(&held,5u,1u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_KEY_A);

    press_and_project(&ux, BLU2USB_CONTROL_KEY_X, &held);
    assert_span(&held,5u,15u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_KEY_X);

    press_and_project(&ux, BLU2USB_CONTROL_KEY_B, &held);
    assert_span(&held,6u,1u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    release_inert(&ux, BLU2USB_CONTROL_KEY_B);
}

static void test_y_locks_on_release_and_unlock_is_consumed(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t held;

    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_SAVED;

    press_and_project(&ux, BLU2USB_CONTROL_KEY_Y, &held);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert_span(&held,6u,15u,5u,BLU2USB_UI_TONE_EMPHASIZED);
    assert_span(&held,8u,1u,11u,BLU2USB_UI_TONE_EMPHASIZED);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_SAVED);

    /* Press alone arms unlock; release completes it and is consumed. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, true);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_SAVED);

    const blu2usb_ux_command_t cmd =
        blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, false);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 0u);
}

static void test_legacy_success_copy_is_absent(void)
{
    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_MOUSE_SAVED);
    assert(screen != NULL);
    for (unsigned row = 0u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        const char *text = screen->rows[row] == NULL ? "" : screen->rows[row];
        assert(strstr(text, "MOUSE PAIRED") == NULL);
        assert(strstr(text, "READY TO USE") == NULL);
    }
}

int main(void)
{
    test_layout_background_and_geometry();
    test_didactic_feedback_and_inert_controls();
    test_y_locks_on_release_and_unlock_is_consumed();
    test_legacy_success_copy_is_absent();
    return 0;
}
