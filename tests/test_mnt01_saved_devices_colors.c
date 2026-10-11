/* MNT-01 / HF-01: the sole Saved Devices action must look selected
 * without changing the established left/right pagination or JOY PRESS
 * removal flow. Run with host assertions enabled. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t press_and_release(blu2usb_ux_model_t *ux,
                                             blu2usb_control_t control)
{
    blu2usb_ux_command_t cmd = blu2usb_ux_input(ux, control, true);
    assert(cmd.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, control, false);
}

static void render(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void assert_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *want)
{
    char text[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned c = 0; c < BLU2USB_RENDERER_TEXT_COLS; ++c)
        text[c] = frame->cells[row][c].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0u && text[n - 1u] == ' ') --n;
    text[n] = '\0';
    assert(strcmp(text, want) == 0);
}

static void assert_colors(const blu2usb_ui_frame_t *frame,
                          bool name_connected, bool bond_exists)
{
    assert(frame->hint_start_row == 7u);
    for (unsigned r = 0u; r < BLU2USB_RENDERER_TEXT_ROWS; ++r) {
        for (unsigned c = 0u; c < BLU2USB_RENDERER_TEXT_COLS; ++c) {
            if (frame->cells[r][c].character == ' ') continue;
            blu2usb_ui_tone_t tone = BLU2USB_UI_TONE_STATIC;
            if (r == 1u && name_connected) tone = BLU2USB_UI_TONE_CURRENT;
            if (r == 5u)
                tone = bond_exists ? BLU2USB_UI_TONE_EMPHASIZED
                                   : BLU2USB_UI_TONE_ACTIONABLE;
            if (r >= 7u) tone = BLU2USB_UI_TONE_ACTIONABLE;
            assert(frame->cells[r][c].tone == tone);
        }
    }
    assert(blu2usb_renderer_tone_rgb565(frame->cells[5][1].tone) ==
           (bond_exists ? BLU2USB_COLOR_WHITE : BLU2USB_COLOR_LIGHT_GRAY));
    assert(blu2usb_renderer_tone_rgb565(frame->cells[7][0].tone) ==
           BLU2USB_COLOR_LIGHT_GRAY);
    assert(blu2usb_renderer_tone_rgb565(frame->cells[8][0].tone) ==
           BLU2USB_COLOR_LIGHT_GRAY);
    assert(blu2usb_renderer_tone_rgb565(frame->cells[2][0].tone) ==
           BLU2USB_COLOR_OFF_WHITE_YELLOW);
    if (name_connected)
        assert(blu2usb_renderer_tone_rgb565(frame->cells[1][0].tone) ==
               BLU2USB_COLOR_CYAN);
}

static void check_saved_screen(blu2usb_ux_model_t *ux,
                               bool current, bool removable)
{
    blu2usb_ui_frame_t frame;
    render(ux, &frame);
    assert_row(&frame, 5u, " REMOVE DEVICE");
    assert_row(&frame, 7u, "JOY RIGHT\\LEFT: PAGE");
    assert_row(&frame, 8u, "JOY PRESS: ACCESS");
    assert_colors(&frame, current, removable);
}

static void open_saved_from_home(blu2usb_ux_model_t *ux)
{
    assert(ux->screen == BLU2USB_SCREEN_HOME);
    assert(press_and_release(ux, BLU2USB_CONTROL_JOY_DOWN).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(press_and_release(ux, BLU2USB_CONTROL_JOY_DOWN).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux->selection == 2u);
    assert(press_and_release(ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux->screen == BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux->selection == 0u);
    assert(blu2usb_ux_option_count(ux) == 0u); /* no new UX list */
}

static void test_connected_multiple_bonds(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 3u);
    blu2usb_ux_set_saved_mouse_name(&ux, 0u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(&ux, 1u, "Mouse Two");
    blu2usb_ux_set_saved_mouse_name(&ux, 2u, "Mouse Three");
    blu2usb_ux_set_current_mouse_name(&ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(&ux, 0);
    open_saved_from_home(&ux);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == 0);
    check_saved_screen(&ux, true, true);

    /* HAT-key highlighting must not override the gray hints or white action. */
    assert(blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_RIGHT, true).kind ==
           BLU2USB_UX_COMMAND_NONE);
    check_saved_screen(&ux, true, true);
    assert(blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_RIGHT, false).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.saved_page == 1u);
    check_saved_screen(&ux, false, true);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 1u) == 1);
    assert(press_and_release(&ux, BLU2USB_CONTROL_JOY_RIGHT).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.saved_page == 2u);
    check_saved_screen(&ux, false, true);
    assert(press_and_release(&ux, BLU2USB_CONTROL_JOY_RIGHT).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.saved_page == 0u);
    check_saved_screen(&ux, true, true);

    assert(press_and_release(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0); /* no wrong-bond action */
    assert(press_and_release(&ux, BLU2USB_CONTROL_KEY_B).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    check_saved_screen(&ux, true, true);
    assert(press_and_release(&ux, BLU2USB_CONTROL_KEY_B).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
}

static void test_disconnected_and_empty(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    blu2usb_ux_set_saved_mouse_name(&ux, 0u, "Logitech Lift");
    open_saved_from_home(&ux);
    check_saved_screen(&ux, false, true);
    assert(press_and_release(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(press_and_release(&ux, BLU2USB_CONTROL_KEY_B).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.saved_page = 0u;
    check_saved_screen(&ux, false, false);
    assert(press_and_release(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux.remove_target_bond == -1);
}

int main(void)
{
    test_connected_multiple_bonds();
    test_disconnected_and_empty();
    return 0;
}
