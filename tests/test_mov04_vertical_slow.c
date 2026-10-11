#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/movement/movement.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t control)
{
    blu2usb_ux_command_t down = blu2usb_ux_input(ux, control, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, control, false);
}

static void project(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void assert_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        actual[col] = frame->cells[row][col].character;
    while (end > 0 && actual[end - 1] == ' ') --end;
    actual[end] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void open_vertical_slow(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_HOME;
    assert(blu2usb_ux_option_count(ux) == 4u);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 1u); /* VERTICAL DPI */
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux->selection == 0u); /* NORMAL 1X */
}

static void test_layout_navigation_and_colors(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_vertical_slow(&ux);
    project(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    assert(blu2usb_ux_option_count(&ux) == 4u);
    static const char *const expected[9] = {
        "VERTICAL SLOW DPI",
        " NORMAL        1x",
        " SLIGHTLY SLOW 0.75x",
        " SLOW          0.5x",
        " VERY SLOW     0.25x",
        "",
        "JOY LEFT\\RIGHT: PAGE",
        "KEY A: APPLY AND BACK",
        "KEY X: HELP SLOW DPI"
    };
    for (unsigned row = 0u; row < 9u; ++row)
        assert_row(&frame, row, expected[row]);
    for (unsigned row = 1u; row <= 4u; ++row)
        assert(frame.cells[row][15].character >= '0' &&
               frame.cells[row][15].character <= '9');
    assert(frame.cells[1][15].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][15].tone == BLU2USB_UI_TONE_ACTIONABLE);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, true);
    assert(ux.selection == 0u); /* no move before release */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, false);
    assert(ux.selection == 3u); /* wrap to VERY SLOW */
    project(&ux, &frame);
    assert(frame.cells[1][15].tone == BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[4][15].tone == BLU2USB_UI_TONE_EMPHASIZED);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
}

static void test_cancel_help_apply_and_axis_independence(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_vertical_slow(&ux);
    blu2usb_movement_settings_t prior = {8u, 4u, 3u}; /* H=2x, rot=270 */
    blu2usb_ux_set_movement(&ux, prior);

    /* Selecting 0.75x and cancelling must not change the confirmed 1x. */
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u);
    blu2usb_ux_command_t result = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(result.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.vertical_quarters == 4u);
    assert(ux.movement_current.horizontal_quarters == 8u);
    assert(ux.movement_current.rotation_quadrants == 3u);

    /* Existing v1.0 screen-entry policy resets parent selection to 0. */
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP); /* VERY SLOW 0.25x */
    assert(ux.selection == 3u);

    /* Fast/Slow pagination must remain within the vertical axis. */
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux.selection == 3u);

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_SLOW_DPI);
    result = tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(result.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux.selection == 3u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, true);
    assert(ux.movement_current.vertical_quarters == 4u);
    result = blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, false);
    assert(result.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(result.movement.vertical_quarters == 1u); /* 0.25x */
    assert(result.movement.horizontal_quarters == 8u); /* unchanged */
    assert(result.movement.rotation_quadrants == 3u); /* unchanged */
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux.movement_current.vertical_quarters == 4u); /* wait for TLV */

    /* Simulate main.c after successful persistent write. */
    blu2usb_ux_set_movement(&ux, result.movement);
    ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection = 1u;
    project(&ux, &frame);
    assert_row(&frame, 1u, " HORIZONTAL DPI 2X");
    assert_row(&frame, 2u, " VERTICAL DPI 0.25X");
    assert_row(&frame, 4u, " ROTATION 270-DEGREE");
    assert(frame.cells[2][14].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[1][16].tone == BLU2USB_UI_TONE_CURRENT);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP); /* candidate 0.5x, saved 0.25x */
    project(&ux, &frame);
    assert(frame.cells[3][15].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[4][15].tone == BLU2USB_UI_TONE_CURRENT);
    result = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(result.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.movement_current.vertical_quarters == 1u);
    assert(ux.movement_current.horizontal_quarters == 8u);
    assert(ux.movement_current.rotation_quadrants == 3u);
}

static void test_four_vertical_gains_and_fractional_accumulator(void)
{
    static const uint8_t quarters[4] = {4u, 3u, 2u, 1u};
    static const char *const values[4] = {
        " VERTICAL DPI 1X",
        " VERTICAL DPI 0.75X",
        " VERTICAL DPI 0.5X",
        " VERTICAL DPI 0.25X"
    };
    for (unsigned select = 0u; select < 4u; ++select) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        open_vertical_slow(&ux);
        for (unsigned i = 0; i < select; ++i)
            tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        const blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(cmd.movement.vertical_quarters == quarters[select]);
        assert(cmd.movement.horizontal_quarters == 4u);
        assert(cmd.movement.rotation_quadrants == 0u);
        blu2usb_ux_set_movement(&ux, cmd.movement);
        ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
        ux.selection = 1u;
        project(&ux, &frame);
        assert_row(&frame, 2u, values[select]);

        /* Four 1-count vertical events produce precisely gain*1 counts,
         * including any fractional carry; horizontal is independent. */
        blu2usb_movement_pipeline_t pipeline = {0};
        int sum_x = 0, sum_y = 0;
        for (unsigned i = 0u; i < 4u; ++i) {
            blu2usb_movement_pipeline_push(&pipeline, ux.movement_current, 1, 1);
            int8_t dx, dy;
            blu2usb_movement_pipeline_next(&pipeline, &dx, &dy);
            sum_x += dx; sum_y += dy;
            blu2usb_movement_pipeline_ack(&pipeline, dx, dy);
        }
        assert(sum_x == 4);
        assert(sum_y == (int)quarters[select]);
        assert(pipeline.pending_x_quarters == 0);
        assert(pipeline.pending_y_quarters == 0);
    }

    /* With H=2x and 270 CCW, gain V must scale only rotated USB_Y. */
    blu2usb_movement_pipeline_t motion = {0};
    const blu2usb_movement_settings_t settings = {8u, 1u, 3u};
    blu2usb_movement_pipeline_push(&motion, settings, 4, 2);
    int8_t x, y;
    blu2usb_movement_pipeline_next(&motion, &x, &y);
    assert(x == -4 && y == 1); /* -2*source_y, source_x*0.25 */
    blu2usb_movement_pipeline_ack(&motion, x, y);
    assert(motion.pending_x_quarters == 0 &&
           motion.pending_y_quarters == 0);
}

int main(void)
{
    test_layout_navigation_and_colors();
    test_cancel_help_apply_and_axis_independence();
    test_four_vertical_gains_and_fractional_accumulator();
    return 0;
}
