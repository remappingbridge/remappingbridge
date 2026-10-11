#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t down = blu2usb_ux_input(ux, key, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
}

static void project(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expect_row(const blu2usb_ui_frame_t *frame, unsigned row, const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        actual[col] = frame->cells[row][col].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0u && actual[n-1] == ' ') --n;
    actual[n] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void expect_tones(const blu2usb_ui_frame_t *frame,
                         bool connected_name, bool removable)
{
    assert(frame->hint_start_row == 7u);
    for (unsigned row = 0; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
            if (frame->cells[row][col].character == ' ') continue;
            blu2usb_ui_tone_t tone = BLU2USB_UI_TONE_STATIC;
            if (row == 1u && connected_name)
                tone = BLU2USB_UI_TONE_CURRENT;
            else if (row == 5u)
                tone = removable ? BLU2USB_UI_TONE_EMPHASIZED
                                 : BLU2USB_UI_TONE_ACTIONABLE;
            else if (row >= 7u)
                tone = BLU2USB_UI_TONE_ACTIONABLE;
            assert(frame->cells[row][col].tone == tone);
        }
    }
    /* MNT-01 supersedes only the old MOV-08 all-yellow action/hints:
     * title/profile/DPI/rotation still yellow, active name still cyan. */
    assert(frame->cells[0][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame->cells[2][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame->cells[3][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame->cells[4][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame->cells[5][1].tone ==
           (removable ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE));
    assert(frame->cells[7][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(frame->cells[8][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_renderer_tone_rgb565(frame->cells[5][1].tone) ==
           (removable ? BLU2USB_COLOR_WHITE : BLU2USB_COLOR_LIGHT_GRAY));
    assert(blu2usb_renderer_tone_rgb565(frame->cells[7][0].tone) ==
           BLU2USB_COLOR_LIGHT_GRAY);
}

static void init_three_saved(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 3u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 2u, "Mouse Gamma");
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux, 1); /* sort front at index 1 */
    blu2usb_ux_set_saved_movement(
        ux, 0u, (blu2usb_movement_settings_t){2u,12u,1u});
    blu2usb_ux_set_saved_movement(
        ux, 2u, (blu2usb_movement_settings_t){5u,6u,2u});
    blu2usb_ux_set_movement(
        ux, (blu2usb_movement_settings_t){8u,3u,3u}); /* active bond 1 */
    ux->active_profile = BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP;
    ux->screen = BLU2USB_SCREEN_HOME;
}

static void open_from_connected_home(blu2usb_ux_model_t *ux)
{
    assert(ux->screen == BLU2USB_SCREEN_HOME);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 2u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_SAVED_DEVICES);
}

static void verify_page(const blu2usb_ux_model_t *ux, unsigned page,
                        int logical_bond, const char *title,
                        const char *name, const char *dpi,
                        const char *rotation, bool connected_name)
{
    blu2usb_ui_frame_t frame;
    assert(ux->screen == BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux->saved_page == page);
    assert(blu2usb_ux_saved_bond_for_page(ux, page) == logical_bond);
    project(ux, &frame);
    expect_row(&frame, 0u, title);
    expect_row(&frame, 1u, name);
    expect_row(&frame, 2u, "PROFILE: PASSTHROUGH");
    expect_row(&frame, 3u, dpi);
    expect_row(&frame, 4u, rotation);
    expect_row(&frame, 5u, " REMOVE DEVICE");
    expect_row(&frame, 6u, "");
    expect_row(&frame, 7u, "JOY RIGHT\\LEFT: PAGE");
    expect_row(&frame, 8u, "JOY PRESS: ACCESS");
    expect_tones(&frame, connected_name, true);
}

static void test_full_page_navigation_and_unique_saved_settings(void)
{
    blu2usb_ux_model_t ux;
    init_three_saved(&ux);
    assert(blu2usb_ux_option_count(&ux) == 4u);
    open_from_connected_home(&ux);
    assert(ux.saved_page == 0u);
    assert(ux.saved_pages == 3u);
    verify_page(&ux, 0u, 1, "1 OF 3", "LOGITECH LIFT MOUSE",
                "DPI: H 2 - V 0.75", "ROTATION: 270 DEGREE", true);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 1u, 0, "2 OF 3", "MOUSE ALPHA",
                "DPI: H 0.5 - V 3", "ROTATION: 90 DEGREE", false);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 2u, 2, "3 OF 3", "MOUSE GAMMA",
                "DPI: H 1.25 - V 1.5", "ROTATION: 180 DEGREE", false);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 0u, 1, "1 OF 3", "LOGITECH LIFT MOUSE",
                "DPI: H 2 - V 0.75", "ROTATION: 270 DEGREE", true);
    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    verify_page(&ux, 2u, 2, "3 OF 3", "MOUSE GAMMA",
                "DPI: H 1.25 - V 1.5", "ROTATION: 180 DEGREE", false);

    /* Pressing a button must never change page, or turn any hint white. */
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_LEFT, true);
    assert(ux.saved_page == 2u);
    blu2usb_ui_frame_t frame;
    project(&ux, &frame);
    expect_tones(&frame, false, true);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_LEFT, false);
    assert(ux.saved_page == 1u);
    verify_page(&ux, 1u, 0, "2 OF 3", "MOUSE ALPHA",
                "DPI: H 0.5 - V 3", "ROTATION: 90 DEGREE", false);

    /* Identity pinned for Remove This is logical bond, not visible page.
       Cancel must retain this page; actual deletion belongs to MOV-09. */
    const blu2usb_ux_command_t press = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(press.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    verify_page(&ux, 1u, 0, "2 OF 3", "MOUSE ALPHA",
                "DPI: H 0.5 - V 3", "ROTATION: 90 DEGREE", false);

    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.movement_current.horizontal_quarters == 8u);
    assert(ux.movement_current.vertical_quarters == 3u);
    assert(ux.movement_current.rotation_quadrants == 3u);
}

static void test_reconnect_changes_front_bond_not_saved_values(void)
{
    blu2usb_ux_model_t ux;
    init_three_saved(&ux);
    open_from_connected_home(&ux);
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_current_mouse_name(&ux, NULL);
    blu2usb_ux_set_saved_connected_bond(&ux, -1);
    assert(ux.saved_front_bond == 1); /* stable page order when disconnected */
    verify_page(&ux, 0u, 1, "1 OF 3", "LOGITECH LIFT MOUSE",
                "DPI: H 2 - V 0.75", "ROTATION: 270 DEGREE", false);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 1u, 0, "2 OF 3", "MOUSE ALPHA",
                "DPI: H 0.5 - V 3", "ROTATION: 90 DEGREE", false);

    /* Simulate main.c restoring connected identity and its TLV payload. */
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(&ux, "Mouse Gamma");
    blu2usb_ux_set_saved_connected_bond(&ux, 2);
    blu2usb_ux_set_movement(
        &ux, (blu2usb_movement_settings_t){5u,6u,2u});
    assert(ux.saved_page == 0u);
    verify_page(&ux, 0u, 2, "1 OF 3", "MOUSE GAMMA",
                "DPI: H 1.25 - V 1.5", "ROTATION: 180 DEGREE", true);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 1u, 0, "2 OF 3", "MOUSE ALPHA",
                "DPI: H 0.5 - V 3", "ROTATION: 90 DEGREE", false);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    verify_page(&ux, 2u, 1, "3 OF 3", "LOGITECH LIFT MOUSE",
                "DPI: H 2 - V 0.75", "ROTATION: 270 DEGREE", false);
    assert(ux.movement_current.horizontal_quarters == 5u);
    assert(ux.movement_current.vertical_quarters == 6u);
    assert(ux.movement_current.rotation_quadrants == 2u);

    /* Global remapping profile isn't historical per-bond state: one current
       runtime value is displayed on every saved page, as in v1.0. */
    ux.active_profile = BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    blu2usb_ui_frame_t frame;
    project(&ux, &frame);
    expect_row(&frame, 2u, "PROFILE: PASSTHROUGH");
    expect_tones(&frame, false, true);
}

static void test_long_names_boundaries_and_zero_saved(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_three_saved(&ux);
    blu2usb_ux_set_saved_mouse_name(
        &ux, 0u, "A Very Long Premium Wireless Ergonomic Mouse Model With Extras");
    open_from_connected_home(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT); /* inactive long name */
    project(&ux, &frame);
    assert(frame.hint_start_row == 7u);
    assert(frame.cells[1][0].character != ' ');
    for (unsigned row = 0u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row)
        for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
            assert(frame.cells[row][col].character != '\0');
    expect_tones(&frame, false, true);
    assert(ux.saved_mouse_names[0][0] == 'A'); /* identity unchanged */

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_saved_connected_bond(&ux, -1);
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.saved_page = 0u;
    project(&ux, &frame);
    expect_row(&frame, 0u, "0 OF 0");
    expect_row(&frame, 1u, "UNKNOWN MOUSE");
    expect_row(&frame, 3u, "DPI: H 1 - V 1");
    expect_row(&frame, 4u, "ROTATION: 0 DEGREE");
    expect_tones(&frame, false, false);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == -1);
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
}

int main(void)
{
    test_full_page_navigation_and_unique_saved_settings();
    test_reconnect_changes_front_bond_not_saved_values();
    test_long_names_boundaries_and_zero_saved();
    return 0;
}
