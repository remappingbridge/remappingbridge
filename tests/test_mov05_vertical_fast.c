#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/movement/movement.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t pressed = blu2usb_ux_input(ux, key, true);
    assert(pressed.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
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
    for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        actual[col] = frame->cells[row][col].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0u && actual[n - 1u] == ' ') --n;
    actual[n] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void start_vertical_fast(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_HOME;
    assert(blu2usb_ux_option_count(ux) == 4u);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);   /* 2nd HOME option */
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);   /* 2nd Movement option */
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux->screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
    assert(ux->selection == 0u);
}

static void test_exact_fast_layout_21x9_selection_and_help(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    start_vertical_fast(&ux);
    project(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    assert(blu2usb_ux_option_count(&ux) == 4u);

    static const char *const rows[9] = {
        "VERTICAL FAST DPI",
        " SLIGHTLY FAST  1.25x",
        " FAST           1.5x",
        " VERY FAST      2x",
        " EXTREMELY FAST 3x",
        "",
        "JOY LEFT\\RIGHT: PAGE",
        "KEY A: APPLY AND BACK",
        "KEY X: HELP FAST DPI"
    };
    for (unsigned row = 0; row < 9u; ++row)
        assert_row(&frame, row, rows[row]);

    /* Values are in one-based column 17 (index 16), fit 21 columns. */
    for (unsigned row = 1u; row <= 4u; ++row) {
        assert(frame.cells[row][16].character >= '0');
        assert(frame.cells[row][16].character <= '9');
        assert(frame.cells[row][16].tone ==
               (row == 1u ? BLU2USB_UI_TONE_EMPHASIZED :
                            BLU2USB_UI_TONE_ACTIONABLE));
    }
    /* Navigation occurs on release and wraps. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, true);
    assert(ux.selection == 0u);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, false);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u);

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_FAST_DPI);
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
    assert(ux.selection == 1u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
    assert(ux.selection == 1u);         /* retains staged Fast selection */
    const blu2usb_ux_command_t cancel = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(cancel.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.vertical_quarters == 4u);
}

static void test_all_four_gains_and_axis_isolation(void)
{
    static const uint8_t gains[4] = {5u, 6u, 8u, 12u};
    static const char *const summary[4] = {
        " VERTICAL DPI 1.25X", " VERTICAL DPI 1.5X",
        " VERTICAL DPI 2X", " VERTICAL DPI 3X"
    };
    for (unsigned desired = 0u; desired < 4u; ++desired) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        start_vertical_fast(&ux);
        const blu2usb_movement_settings_t initial = {3u, 4u, 2u};
        blu2usb_ux_set_movement(&ux, initial); /* H=0.75x, ROT=180 */
        for (unsigned i = 0u; i < desired; ++i)
            tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        assert(ux.selection == desired);
        const blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(cmd.movement.horizontal_quarters == 3u);
        assert(cmd.movement.vertical_quarters == gains[desired]);
        assert(cmd.movement.rotation_quadrants == 2u);

        /* Before the real TLV write is confirmed, the UI is not 'applied'. */
        assert(ux.screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
        assert(ux.movement_current.vertical_quarters == 4u);

        blu2usb_ux_set_movement(&ux, cmd.movement);
        ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
        ux.selection = 1u;
        project(&ux, &frame);
        assert_row(&frame, 1u, " HORIZONTAL DPI 0.75X");
        assert_row(&frame, 2u, summary[desired]);
        assert_row(&frame, 4u, " ROTATION 180-DEGREE");
        assert(frame.cells[2][14].tone == BLU2USB_UI_TONE_EMPHASIZED);

        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        assert(ux.screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI);
        assert(ux.selection == desired);
        project(&ux, &frame);
        assert(frame.cells[desired + 1u][16].tone ==
               BLU2USB_UI_TONE_EMPHASIZED);
        tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        project(&ux, &frame);
        assert(frame.cells[desired + 1u][16].tone ==
               BLU2USB_UI_TONE_CURRENT);
        assert(frame.cells[ux.selection + 1u][16].tone ==
               BLU2USB_UI_TONE_EMPHASIZED);
        tap(&ux, BLU2USB_CONTROL_KEY_B);
        assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
        assert(ux.movement_current.vertical_quarters == gains[desired]);
        assert(ux.movement_current.horizontal_quarters == 3u);
        assert(ux.movement_current.rotation_quadrants == 2u);
    }
}

static void test_usb_relative_fraction_accumulation_and_rotated_axes(void)
{
    const uint8_t gains[4] = {5u, 6u, 8u, 12u};
    for (unsigned g = 0; g < 4u; ++g) {
        const blu2usb_movement_settings_t settings = {3u, gains[g], 0u};
        blu2usb_movement_pipeline_t p = {0};
        int sum_x = 0, sum_y = 0;
        /* Four 1-count diagonal events: X=0.75x and Y=Fast, exact. */
        for (unsigned i = 0; i < 4u; ++i) {
            blu2usb_movement_pipeline_push(&p, settings, 1, 1);
            int8_t dx,dy;
            blu2usb_movement_pipeline_next(&p, &dx, &dy);
            sum_x += dx;
            sum_y += dy;
            blu2usb_movement_pipeline_ack(&p, dx, dy);
        }
        assert(sum_x == 3);
        assert(sum_y == (int)gains[g]);
        assert(p.pending_x_quarters == 0);
        assert(p.pending_y_quarters == 0);
    }
    blu2usb_movement_pipeline_t rotated = {0};
    /* CCW 270 -> USB X=-Y; USB Y=X. V=3x only affects output Y. */
    const blu2usb_movement_settings_t settings = {4u, 12u, 3u};
    blu2usb_movement_pipeline_push(&rotated, settings, 3, 2);
    int8_t x, y;
    blu2usb_movement_pipeline_next(&rotated, &x, &y);
    assert(x == -2 && y == 9);
    blu2usb_movement_pipeline_ack(&rotated, x, y);
    assert(rotated.pending_x_quarters == 0);
    assert(rotated.pending_y_quarters == 0);
    /* Repeat a large event; each USB report is limited to signed int8
       and unsent distance remains in the pipeline until acknowledged. */
    blu2usb_movement_pipeline_push(&rotated, settings, 400, 300);
    int64_t total_x = 0, total_y = 0;
    for (unsigned n = 0; n < 16u; ++n) {
        blu2usb_movement_pipeline_next(&rotated, &x, &y);
        if (!x && !y) break;
        total_x += x;
        total_y += y;
        blu2usb_movement_pipeline_ack(&rotated, x, y);
    }
    assert(total_x == -300 && total_y == 1200);
    assert(rotated.pending_x_quarters == 0);
    assert(rotated.pending_y_quarters == 0);
}

int main(void)
{
    test_exact_fast_layout_21x9_selection_and_help();
    test_all_four_gains_and_axis_isolation();
    test_usb_relative_fraction_accumulation_and_rotated_axes();
    return 0;
}
