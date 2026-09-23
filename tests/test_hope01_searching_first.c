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

static void assert_visible_row_tone(const blu2usb_ui_frame_t *frame,
                                    unsigned row,
                                    blu2usb_ui_tone_t tone)
{
    bool visible = false;
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
        if (frame->cells[row][col].character == ' ') continue;
        visible = true;
        assert(frame->cells[row][col].tone == tone);
    }
    assert(visible);
}

static void assert_span_tone(const blu2usb_ui_frame_t *frame,
                             unsigned row, unsigned col, unsigned count,
                             blu2usb_ui_tone_t tone)
{
    for (unsigned i = 0u; i < count; ++i)
        assert(frame->cells[row][col + i].tone == tone);
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void test_exact_screen_and_palette(void)
{
    static const char *const expected[BLU2USB_RENDERER_TEXT_ROWS] = {
        "SEARCHING FIRST MOUSE",
        "PRESS TO LEARN KEYS",
        "WHILE WAIT CONNECTION",
        "       JOY UP",
        "  JOY    JOY    JOY",
        "  LEFT  PRESS  RIGHT",
        "      JOY DOWN",
        " KEY A         KEY X",
        " KEY B         KEY Y",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_LEARN_KEYS;
    project(&ux, &frame);

    assert(frame.learn_background);
    assert(frame.hint_start_row == BLU2USB_RENDERER_TEXT_ROWS);
    for (unsigned row = 0u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        row_text(&frame, row, actual);
        assert(strcmp(actual, expected[row]) == 0);
        assert(blu2usb_renderer_background_rgb565(&frame, (uint8_t)row) ==
               BLU2USB_COLOR_DARK_MAGENTA);
    }

    assert_visible_row_tone(&frame, 0u, BLU2USB_UI_TONE_TITLE);
    assert_visible_row_tone(&frame, 1u, BLU2USB_UI_TONE_STATIC);
    assert_visible_row_tone(&frame, 2u, BLU2USB_UI_TONE_STATIC);
    for (unsigned row = 3u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row)
        assert_visible_row_tone(&frame, row, BLU2USB_UI_TONE_ACTIONABLE);

    assert(blu2usb_renderer_text_y(&frame, 0u) == 8u);
    assert(blu2usb_renderer_text_y(&frame, 1u) == 39u);
    assert(blu2usb_renderer_text_y(&frame, 2u) == 64u);
    assert(blu2usb_renderer_text_y(&frame, 8u) == 214u);
}

static void press_project_release(blu2usb_ux_model_t *ux,
                                  blu2usb_control_t control,
                                  blu2usb_ui_frame_t *held,
                                  blu2usb_ui_frame_t *released)
{
    (void)blu2usb_ux_input(ux, control, true);
    assert(ux->screen == BLU2USB_SCREEN_LEARN_KEYS);
    assert(!blu2usb_interaction_is_locked(&ux->interaction));
    project(ux, held);

    (void)blu2usb_ux_input(ux, control, false);
    assert(ux->screen == BLU2USB_SCREEN_LEARN_KEYS);
    assert(!blu2usb_interaction_is_locked(&ux->interaction));
    project(ux, released);
}

static void test_didactic_press_feedback(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t held, released;

    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_LEARN_KEYS;

    press_project_release(&ux, BLU2USB_CONTROL_JOY_UP, &held, &released);
    assert_span_tone(&held, 3u, 7u, 6u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_span_tone(&released, 3u, 7u, 6u, BLU2USB_UI_TONE_ACTIONABLE);

    press_project_release(&ux, BLU2USB_CONTROL_JOY_LEFT, &held, &released);
    assert_span_tone(&held, 4u, 2u, 3u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_span_tone(&held, 5u, 2u, 4u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_JOY_PRESS, &held, &released);
    assert_span_tone(&held, 4u, 9u, 3u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_span_tone(&held, 5u, 8u, 5u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_JOY_RIGHT, &held, &released);
    assert_span_tone(&held, 4u, 16u, 3u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_span_tone(&held, 5u, 15u, 5u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_JOY_DOWN, &held, &released);
    assert_span_tone(&held, 6u, 6u, 8u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_KEY_A, &held, &released);
    assert_span_tone(&held, 7u, 1u, 5u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_KEY_X, &held, &released);
    assert_span_tone(&held, 7u, 15u, 5u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_KEY_B, &held, &released);
    assert_span_tone(&held, 8u, 1u, 5u, BLU2USB_UI_TONE_EMPHASIZED);

    press_project_release(&ux, BLU2USB_CONTROL_KEY_Y, &held, &released);
    assert_span_tone(&held, 8u, 15u, 5u, BLU2USB_UI_TONE_EMPHASIZED);
}

static void test_every_control_is_inert(void)
{
    for (unsigned raw = 0u; raw < BLU2USB_CONTROL_COUNT; ++raw) {
        blu2usb_ux_model_t ux;
        blu2usb_ux_init(&ux);
        ux.screen = BLU2USB_SCREEN_LEARN_KEYS;

        const blu2usb_control_t control = (blu2usb_control_t)raw;
        const blu2usb_ux_command_t press = blu2usb_ux_input(&ux, control, true);
        const blu2usb_ux_command_t release = blu2usb_ux_input(&ux, control, false);

        assert(press.kind == BLU2USB_UX_COMMAND_NONE);
        assert(release.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
    }
}

int main(void)
{
    test_exact_screen_and_palette();
    test_didactic_press_feedback();
    test_every_control_is_inert();
    return 0;
}
