#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t pressed = blu2usb_ux_input(ux, key, true);
    assert(pressed.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
}

static void render(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expect_rows(const blu2usb_ui_frame_t *frame,
                        const char *const expected[9])
{
    assert(frame->hint_start_row == 8u);
    for (unsigned row = 0u; row < BLU2USB_RENDERER_TEXT_ROWS; ++row) {
        char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
        unsigned end = BLU2USB_RENDERER_TEXT_COLS;
        for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
            actual[col] = frame->cells[row][col].character;
        while (end > 0u && actual[end - 1u] == ' ') --end;
        actual[end] = '\0';
        assert(strcmp(actual, expected[row]) == 0);
    }
}

static const char *const slow_rows[9] = {
    "SLOW DPI HELP", "SLOWER DPI POINTER", "ARE RECOMMENDED",
    "FOR MOVEMENTS THAT", "REQUIRE GREATER", "SENSITIVITY AND",
    "PRECISION", "", "ANY KEY: BACK"
};

static const char *const fast_rows[9] = {
    "FAST DPI HELP", "FASTER DPI POINTER", "IS RECOMMENDED FOR",
    "HIGH-RESOLUTION", "DESKTOP AND", "WIDESCREEN",
    "ENVIRONMENTS", "", "ANY KEY: BACK"
};

static const char *const rotation_rows[9] = {
    "ROTATION HELP", "MOUSE MOVEMENT", "CHANGES DIRECTION",
    "EVERY 90 DEGREES", "FROM THE PREVIOUS", "DIRECTION",
    "ANTICLOCKWISE", "", "ANY KEY: BACK"
};

static const char *const remove_rows[9] = {
    "REMOVE MOUSE HELP", "COMPLETELY REMOVE THE", "AUTOMATIC CONNECTION",
    "WHEN TURNING ON THE", "DEVICE AND DELETE ITS", "BUTTON REMAPPING AND",
    "CUSTOM MOVEMENTS", "", "ANY KEY: BACK"
};

static void connected_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux, 1);
    ux->screen = BLU2USB_SCREEN_HOME;
    assert(blu2usb_ux_option_count(ux) == 4u);
}

static void open_movement(blu2usb_ux_model_t *ux)
{
    connected_home(ux);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 1u);
    assert(tap(ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
}

static void open_dpi(blu2usb_ux_model_t *ux, bool vertical, bool fast)
{
    open_movement(ux);
    if (vertical) tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == (vertical ? 1u : 0u));
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == (vertical ? BLU2USB_SCREEN_VERTICAL_SLOW_DPI :
                                     BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI));
    if (fast) {
        tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
        assert(ux->screen == (vertical ? BLU2USB_SCREEN_VERTICAL_FAST_DPI :
                                         BLU2USB_SCREEN_HORIZONTAL_FAST_DPI));
    }
}

static void test_all_dpi_help_shared_text_and_non_locking_return(void)
{
    for (unsigned axis = 0u; axis < 2u; ++axis) {
        for (unsigned speed = 0u; speed < 2u; ++speed) {
            blu2usb_ux_model_t ux;
            blu2usb_ui_frame_t frame;
            const bool vertical = axis != 0u;
            const bool fast = speed != 0u;
            open_dpi(&ux, vertical, fast);
            const blu2usb_screen_id_t origin = ux.screen;
            const unsigned desired = fast ? 3u : 2u;
            for (unsigned i = 0u; i < desired; ++i)
                tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
            assert(ux.selection == desired);

            const blu2usb_movement_settings_t unchanged = ux.movement_current;
            /* Only the RELEASE of KEY X enters Help. */
            (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_X, true);
            assert(ux.screen == origin);
            const blu2usb_ux_command_t help =
                blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_X, false);
            assert(help.kind == BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen == (fast ? BLU2USB_SCREEN_HELP_FAST_DPI :
                                      BLU2USB_SCREEN_HELP_SLOW_DPI));
            assert(ux.return_screen == origin);
            assert(ux.return_selection == desired);
            render(&ux, &frame);
            expect_rows(&frame, fast ? fast_rows : slow_rows);

            /* KEY Y is Any Key: Back in Help; it must NOT lock LCD. */
            assert(tap(&ux, BLU2USB_CONTROL_KEY_Y).kind == BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen == origin);
            assert(ux.selection == desired);
            assert(!blu2usb_interaction_is_locked(&ux.interaction));
            assert(memcmp(&ux.movement_current, &unchanged,
                          sizeof(unchanged)) == 0);

            /* Returning with KEY A, JOY PRESS, or B is consumed only by
               Help and cannot APPLY, change candidates or navigate. */
            tap(&ux, BLU2USB_CONTROL_KEY_X);
            assert(ux.screen == (fast ? BLU2USB_SCREEN_HELP_FAST_DPI :
                                      BLU2USB_SCREEN_HELP_SLOW_DPI));
            assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen == origin && ux.selection == desired);
            assert(memcmp(&ux.movement_current, &unchanged,
                          sizeof(unchanged)) == 0);
            tap(&ux, BLU2USB_CONTROL_KEY_X);
            assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
                   BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen == origin && ux.selection == desired);
            tap(&ux, BLU2USB_CONTROL_KEY_X);
            assert(tap(&ux, BLU2USB_CONTROL_KEY_B).kind ==
                   BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen == origin && ux.selection == desired);

            /* Only a new explicit B on original page cancels the candidate. */
            tap(&ux, BLU2USB_CONTROL_KEY_B);
            assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
        }
    }
}

static void test_rotation_help_exact_copy_and_current_selection(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    open_movement(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);

    const blu2usb_movement_settings_t initial = ux.movement_current;
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_ROTATION);
    render(&ux, &frame);
    expect_rows(&frame, rotation_rows);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);
    assert(ux.selection == 3u);
    assert(ux.movement_current.rotation_quadrants == initial.rotation_quadrants);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_Y).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_ROTATION);
    assert(ux.selection == 3u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
}

static void test_remove_help_maintains_logical_bond_and_pending_safety(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    connected_home(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 1u);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 1u) == 0);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);
    assert(!ux.remove_pending);

    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMOVE_THIS);
    render(&ux, &frame);
    expect_rows(&frame, remove_rows);
    /* Even KEY A from help is consumed; it must not REMOVE. */
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);
    assert(!ux.remove_pending);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_Y).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    /* The confirmation page still safely guards Help while deleting. */
    const blu2usb_ux_command_t remove = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(remove.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(remove.saved_bond == 0);
    assert(ux.remove_pending);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_Y).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
}

int main(void)
{
    test_all_dpi_help_shared_text_and_non_locking_return();
    test_rotation_help_exact_copy_and_current_selection();
    test_remove_help_maintains_logical_bond_and_pending_safety();
    return 0;
}
