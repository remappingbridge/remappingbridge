#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    blu2usb_ux_command_t pressed = blu2usb_ux_input(ux, key, true);
    assert(pressed.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
}

static void draw(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char out[BLU2USB_RENDERER_TEXT_COLS + 1u])
{
    for (unsigned i = 0; i < BLU2USB_RENDERER_TEXT_COLS; ++i)
        out[i] = frame->cells[row][i].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0 && out[n - 1] == ' ') --n;
    out[n] = '\0';
}

static void assert_row(const blu2usb_ui_frame_t *f, unsigned row, const char *text)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    row_text(f, row, actual);
    assert(strcmp(actual, text) == 0);
}

static void connected_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Mouse Alpha");
    blu2usb_ux_set_saved_connected_bond(ux, 0);
    ux->screen = BLU2USB_SCREEN_HOME;
}

static void open_fast(blu2usb_ux_model_t *ux)
{
    tap(ux, BLU2USB_CONTROL_JOY_DOWN); /* Movement is second HOME option */
    assert(ux->selection == 1u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux->screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
}

static void test_grid_wrap_help_and_cancel(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    connected_home(&ux);
    open_fast(&ux);

    draw(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    static const char *const expected[9] = {
        "HORIZONTAL FAST DPI",
        " SLIGHTLY FAST  1.25x",
        " FAST           1.5x",
        " VERY FAST      2x",
        " EXTREMELY FAST 3x",
        "",
        "JOY LEFT\\RIGHT: PAGE",
        "KEY A: APPLY AND BACK",
        "KEY X: HELP FAST DPI"
    };
    for (unsigned row = 0; row < 9; ++row) assert_row(&frame, row, expected[row]);

    for (unsigned row = 1; row <= 4; ++row) {
        assert(frame.cells[row][16].character != ' ');
        assert(frame.cells[row][16].tone == (row == 1 ?
            BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE));
    }

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_FAST_DPI);
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
    assert(ux.selection == 1u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
    assert(ux.selection == 1u);

    blu2usb_ux_command_t cancel = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(cancel.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(ux.movement_current.horizontal_quarters == 4u);
}

static void test_fast_gains_and_independent_axis(void)
{
    static const uint8_t expected_gains[4] = {5u, 6u, 8u, 12u};
    static const char *const summaries[4] = {
        " HORIZONTAL DPI 1.25X", " HORIZONTAL DPI 1.5X",
        " HORIZONTAL DPI 2X", " HORIZONTAL DPI 3X"
    };
    for (unsigned selected = 0u; selected < 4u; ++selected) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        connected_home(&ux);
        open_fast(&ux);
        for (unsigned i = 0u; i < selected; ++i)
            tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        assert(ux.selection == selected);

        blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
        assert(cmd.movement.horizontal_quarters == expected_gains[selected]);
        assert(cmd.movement.vertical_quarters == 4u);
        assert(cmd.movement.rotation_quadrants == 0u);
        /* Firmware must confirm TLV storage before changing UI/current value. */
        assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
        assert(ux.movement_current.horizontal_quarters == 4u);

        blu2usb_ux_set_movement(&ux, cmd.movement);
        ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
        ux.selection = 0u;
        draw(&ux, &frame);
        assert_row(&frame, 1u, summaries[selected]);
        assert(frame.cells[1][16].tone == BLU2USB_UI_TONE_CURRENT);
        assert_row(&frame, 2u, " VERTICAL DPI 1X");

        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        assert(ux.screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
        assert(ux.selection == selected);
        draw(&ux, &frame);
        assert(frame.cells[selected + 1u][16].tone ==
               BLU2USB_UI_TONE_EMPHASIZED);

        /* The previously applied value remains cyan if the cursor moves. */
        tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        draw(&ux, &frame);
        assert(frame.cells[selected + 1u][16].tone ==
               BLU2USB_UI_TONE_CURRENT);
        assert(frame.cells[1u + ux.selection][16].tone ==
               BLU2USB_UI_TONE_EMPHASIZED);
        tap(&ux, BLU2USB_CONTROL_KEY_B);
        assert(ux.movement_current.horizontal_quarters == expected_gains[selected]);
    }
}

static void assert_page_tones(const blu2usb_ux_model_t *ux, bool mouse_active)
{
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(ux, &frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, &frame);
    for (unsigned row = 0; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
            if (frame.cells[row][col].character == ' ') continue;
            const blu2usb_ui_tone_t required =
                row == 1u && mouse_active ? BLU2USB_UI_TONE_CURRENT :
                    BLU2USB_UI_TONE_STATIC;
            assert(frame.cells[row][col].tone == required);
        }
    }
    assert(frame.cells[0][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[2][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[3][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[4][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[5][1].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[7][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[8][0].tone == BLU2USB_UI_TONE_STATIC);
}

static void test_saved_devices_identity_only_cyan_on_every_page(void)
{
    blu2usb_ux_model_t ux;
    connected_home(&ux);
    blu2usb_ux_set_saved_movement(&ux, 0u,
        (blu2usb_movement_settings_t){8u, 3u, 3u});
    blu2usb_ux_set_saved_movement(&ux, 1u,
        (blu2usb_movement_settings_t){2u, 12u, 1u});
    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    assert(ux.saved_page == 0u);
    assert_page_tones(&ux, true);

    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 1u);
    assert_page_tones(&ux, false);
    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.saved_page == 0u);
    assert_page_tones(&ux, true);

    /* No matter which navigation control is physically pressed, the
     * selected item/hints must remain the standard yellow. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_RIGHT, true);
    assert_page_tones(&ux, true);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_RIGHT, false);

    /* After disconnection the former connected mouse name loses cyan. */
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_saved_connected_bond(&ux, -1);
    assert_page_tones(&ux, false);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert_page_tones(&ux, false);

    /* Reconnection of another bonded mouse moves the sole cyan name. */
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_saved_connected_bond(&ux, 1);
    assert(ux.saved_page == 0u);
    assert_page_tones(&ux, true);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert_page_tones(&ux, false);
}

int main(void)
{
    test_grid_wrap_help_and_cancel();
    test_fast_gains_and_independent_axis();
    test_saved_devices_identity_only_cyan_on_every_page();
    return 0;
}
