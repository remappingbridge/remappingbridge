#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/movement/movement.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t down = blu2usb_ux_input(ux, key, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
}

static void draw(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expect_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        actual[col] = frame->cells[row][col].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0u && actual[n - 1u] == ' ') --n;
    actual[n] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void connected_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_HOME;
    assert(blu2usb_ux_option_count(ux) == 4u);
}

static void open_rotation(blu2usb_ux_model_t *ux)
{
    assert(ux->screen == BLU2USB_SCREEN_HOME);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 2u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_ROTATION);
}

static void test_layout_selection_help_and_cancel(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    connected_home(&ux);
    open_rotation(&ux);

    assert(blu2usb_ux_option_count(&ux) == 4u);
    assert(ux.selection == 0u);
    draw(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    static const char *const rows[9] = {
        "ROTATION",
        " 0-DEGREE", " 90-DEGREE", " 180-DEGREE", " 270-DEGREE",
        "", "KEY A: APPLY AND BACK", "KEY B: CANCEL", "KEY X: HELP"
    };
    for (unsigned row = 0; row < 9u; ++row)
        expect_row(&frame, row, rows[row]);
    assert(strlen(rows[6]) == BLU2USB_RENDERER_TEXT_COLS); /* exact 21-column hint */
    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_ACTIONABLE);

    /* Down state is visual feedback only; navigation on release. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, true);
    assert(ux.selection == 0u);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_UP, false);
    assert(ux.selection == 3u); /* wraps to 270 */
    draw(&ux, &frame);
    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[4][1].tone == BLU2USB_UI_TONE_EMPHASIZED);

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_ROTATION);
    draw(&ux, &frame);
    expect_row(&frame, 0u, "ROTATION HELP");
    expect_row(&frame, 6u, "ANTICLOCKWISE");
    expect_row(&frame, 8u, "ANY KEY: BACK");
    tap(&ux, BLU2USB_CONTROL_KEY_Y); /* Help must not invoke global Lock */
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);
    assert(ux.selection == 3u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    const blu2usb_ux_command_t cancelled = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(cancelled.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.rotation_quadrants == 0u);
    assert(ux.movement_current.horizontal_quarters == 4u);
    assert(ux.movement_current.vertical_quarters == 4u);
}

static void test_four_angles_apply_only_after_storage_and_reset(void)
{
    static const char *const titles[4] = {
        " ROTATION 0-DEGREE", " ROTATION 90-DEGREE",
        " ROTATION 180-DEGREE", " ROTATION 270-DEGREE"
    };
    for (unsigned angle = 0u; angle < 4u; ++angle) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        connected_home(&ux);
        blu2usb_ux_set_movement(&ux,
                                (blu2usb_movement_settings_t){8u, 3u, 0u});
        open_rotation(&ux);
        for (unsigned i = 0u; i < angle; ++i)
            tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        assert(ux.selection == angle);
        const blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(cmd.movement.rotation_quadrants == angle);
        assert(cmd.movement.horizontal_quarters == 8u);
        assert(cmd.movement.vertical_quarters == 3u);
        assert(ux.screen == BLU2USB_SCREEN_ROTATION); /* no early success */
        assert(ux.movement_current.rotation_quadrants == 0u);

        /* main.c acknowledges movement write before replacing the screen. */
        blu2usb_ux_set_movement(&ux, cmd.movement);
        ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
        ux.selection = 2u;
        draw(&ux, &frame);
        expect_row(&frame, 1u, " HORIZONTAL DPI 2");
        expect_row(&frame, 2u, " VERTICAL DPI 0.75");
        expect_row(&frame, 4u, titles[angle]);
        assert(frame.cells[4][10].tone == BLU2USB_UI_TONE_EMPHASIZED);

        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        assert(ux.screen == BLU2USB_SCREEN_ROTATION);
        assert(ux.selection == angle); /* applied angle preselected */
        tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        draw(&ux, &frame);
        assert(frame.cells[1u + angle][1].tone == BLU2USB_UI_TONE_CURRENT);
        assert(frame.cells[1u + ux.selection][1].tone ==
               BLU2USB_UI_TONE_EMPHASIZED);
        tap(&ux, BLU2USB_CONTROL_KEY_B); /* never persists candidate */
        assert(ux.movement_current.rotation_quadrants == angle);

        /* RESET ALL on Movement Options resets both gains and angle
         * in one command and does not update applied fields until TLV ACK. */
        const blu2usb_ux_command_t reset = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(reset.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(reset.movement.horizontal_quarters == 4u);
        assert(reset.movement.vertical_quarters == 4u);
        assert(reset.movement.rotation_quadrants == 0u);
        assert(ux.movement_current.horizontal_quarters == 8u);
        assert(ux.movement_current.vertical_quarters == 3u);
        assert(ux.movement_current.rotation_quadrants == angle);
        blu2usb_ux_set_movement(&ux, reset.movement);
        assert(ux.movement_current.horizontal_quarters == 4u);
        assert(ux.movement_current.vertical_quarters == 4u);
        assert(ux.movement_current.rotation_quadrants == 0u);
    }
}

static void assert_vector(blu2usb_movement_settings_t settings,
                          int32_t in_x, int32_t in_y,
                          int64_t expected_x, int64_t expected_y)
{
    blu2usb_movement_pipeline_t p = {0};
    blu2usb_movement_pipeline_push(&p, settings, in_x, in_y);
    int64_t sum_x = 0, sum_y = 0;
    for (unsigned i = 0u; i < 100000u; ++i) {
        int8_t out_x = 0, out_y = 0;
        blu2usb_movement_pipeline_next(&p, &out_x, &out_y);
        if (out_x == 0 && out_y == 0) break;
        sum_x += out_x;
        sum_y += out_y;
        blu2usb_movement_pipeline_ack(&p, out_x, out_y);
    }
    assert(sum_x == expected_x);
    assert(sum_y == expected_y);
    assert(p.pending_x_quarters == 0);
    assert(p.pending_y_quarters == 0);
}

static void test_all_angles_and_axis_gains_on_real_output_frame(void)
{
    /* CCW in output screen coordinates, with screen Y positive downward.
       H=2x, V=1x, raw input x=7/y=11:
       0  => (+2*x, +y)
       90 => (+2*y, -x)
       180 => (-2*x, -y)
       270 => (-2*y, +x), physically validated prior G06 wiring. */
    assert_vector((blu2usb_movement_settings_t){8u,4u,0u},7,11,14,11);
    assert_vector((blu2usb_movement_settings_t){8u,4u,1u},7,11,22,-7);
    assert_vector((blu2usb_movement_settings_t){8u,4u,2u},7,11,-14,-11);
    assert_vector((blu2usb_movement_settings_t){8u,4u,3u},7,11,-22,7);

    /* Inputs map onto USB output axes before independent gains apply. */
    assert_vector((blu2usb_movement_settings_t){8u,4u,3u},100,50,-100,100);
    assert_vector((blu2usb_movement_settings_t){4u,12u,3u},100,50,-50,300);
    assert_vector((blu2usb_movement_settings_t){5u,3u,3u},4,-8,10,3);

    /* Signed +/- USB chunks drain full large movement without clipping. */
    assert_vector((blu2usb_movement_settings_t){8u,12u,3u},400,300,-600,1200);
    assert_vector((blu2usb_movement_settings_t){8u,12u,1u},400,300,600,-1200);

    /* Four fractional 1-count moves retain subpixel carry in both axes
       after rotation and do not rotate buttons/wheel (which aren't part of
       this vector module). */
    blu2usb_movement_pipeline_t p = {0};
    const blu2usb_movement_settings_t settings = {3u,5u,3u};
    int total_x = 0, total_y = 0;
    for (unsigned i = 0u; i < 4u; ++i) {
        blu2usb_movement_pipeline_push(&p, settings, 1, 1);
        int8_t x, y;
        blu2usb_movement_pipeline_next(&p, &x, &y);
        total_x += x; total_y += y;
        blu2usb_movement_pipeline_ack(&p, x, y);
    }
    assert(total_x == -3 && total_y == 5);
    assert(p.pending_x_quarters == 0 && p.pending_y_quarters == 0);
}

int main(void)
{
    test_layout_selection_help_and_cancel();
    test_four_angles_apply_only_after_storage_and_reset();
    test_all_angles_and_axis_gains_on_real_output_frame();
    return 0;
}
