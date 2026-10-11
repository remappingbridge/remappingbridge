#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t control)
{
    blu2usb_ux_command_t down = blu2usb_ux_input(ux, control, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, control, false);
}

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char out[BLU2USB_RENDERER_TEXT_COLS + 1u])
{
    for (unsigned column = 0; column < BLU2USB_RENDERER_TEXT_COLS; ++column)
        out[column] = frame->cells[row][column].character;
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    while (end > 0u && out[end - 1u] == ' ') --end;
    out[end] = '\0';
}

static void expect_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    row_text(frame, row, actual);
    assert(strcmp(actual, expected) == 0);
}

static void project(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void open_slow_from_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_HOME;
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
}

static void test_exact_grid_and_candidate_colors(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_slow_from_home(&ux);

    assert(blu2usb_ux_option_count(&ux) == 4u);
    assert(ux.selection == 0u);
    project(&ux, &frame);
    assert(frame.hint_start_row == 6u);

    static const char *const expected[9] = {
        "HORIZONTAL SLOW DPI",
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
        expect_row(&frame, row, expected[row]);

    /* Values start at one-based column 16, never beyond column 21. */
    for (unsigned row = 1u; row <= 4u; ++row) {
        assert(frame.cells[row][14].character == ' ');
        assert(frame.cells[row][15].character >= '0' &&
               frame.cells[row][15].character <= '9');
    }
    assert(frame.cells[1][15].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][15].tone == BLU2USB_UI_TONE_ACTIONABLE);

    /* DOWN only acts after release, and the confirmed 1x stays cyan. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, true);
    assert(ux.selection == 0u);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, false);
    assert(ux.selection == 1u);
    project(&ux, &frame);
    assert(frame.cells[1][15].tone == BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[2][15].tone == BLU2USB_UI_TONE_EMPHASIZED);

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
}

static void test_cancel_help_apply_persisted_state_and_isolation(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_slow_from_home(&ux);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN); /* stage 0.75x */
    blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.horizontal_quarters == 4u);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 2u); /* 0.5x */

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_SLOW_DPI);
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.selection == 2u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, true);
    assert(ux.movement_current.horizontal_quarters == 4u);
    cmd = blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, false);
    assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.horizontal_quarters == 2u);
    assert(cmd.movement.vertical_quarters == 4u);
    assert(cmd.movement.rotation_quadrants == 0u);
    /* Until storage confirmation, neither page nor cyan value changes. */
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.movement_current.horizontal_quarters == 4u);

    /* Simulate main.c acknowledgement after successful TLV persistence. */
    blu2usb_ux_set_movement(&ux, cmd.movement);
    ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection = 0u;
    project(&ux, &frame);
    expect_row(&frame, 1u, " HORIZONTAL DPI 0.5");
    expect_row(&frame, 2u, " VERTICAL DPI 1");

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP); /* stage 0.75x without applying */
    project(&ux, &frame);
    assert(frame.cells[2][15].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[3][15].tone == BLU2USB_UI_TONE_CURRENT);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.horizontal_quarters == 2u);
}

static void test_rotation_hint_and_release_semantics(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_slow_from_home(&ux);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);

    const blu2usb_screen_template_t *rotation =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_ROTATION);
    assert(rotation != NULL);
    assert(strcmp(rotation->rows[6], "KEY A: APPLY AND BACK") == 0);
    assert(strlen(rotation->rows[6]) == BLU2USB_RENDERER_TEXT_COLS);
    project(&ux, &frame);
    expect_row(&frame, 6u, "KEY A: APPLY AND BACK");
    assert(frame.hint_start_row == 6u);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u); /* 90-degrees */
    const blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.rotation_quadrants == 1u);
    /* Firmware goes back only after confirmed persistence. */
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);
}

int main(void)
{
    test_exact_grid_and_candidate_colors();
    test_cancel_help_apply_persisted_state_and_isolation();
    test_rotation_hint_and_release_semantics();
    return 0;
}
