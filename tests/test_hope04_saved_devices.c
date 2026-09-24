#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux,
                                blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char out[BLU2USB_RENDERER_TEXT_COLS + 1u])
{
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        out[col] = frame->cells[row][col].character;
    while (end > 0u && out[end - 1u] == ' ') --end;
    out[end] = '\0';
}

static void assert_row(const blu2usb_ui_frame_t *frame,
                       unsigned row, const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    row_text(frame, row, actual);
    assert(strcmp(actual, expected) == 0);
}

static void assert_tone(const blu2usb_ui_frame_t *frame,
                        unsigned row, blu2usb_ui_tone_t tone)
{
    bool found = false;
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
        if (frame->cells[row][col].character == ' ') continue;
        found = true;
        assert(frame->cells[row][col].tone == tone);
    }
    assert(found);
}

static void init_saved(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 3u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 2u, "Mouse Gamma");
    ux->active_profile = BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP;
    ux->screen = BLU2USB_SCREEN_SAVED_DEVICES;
}

static void test_connected_mouse_is_always_page_one(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_saved(&ux);
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(&ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(&ux, 1);

    assert(ux.saved_page == 0u);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == 1);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 1u) == 0);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 2u) == 2);

    project(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    assert_row(&frame, 0u, "1 OF 3");
    assert_row(&frame, 1u, "LOGITECH LIFT MOUSE");
    assert_tone(&frame, 1u, BLU2USB_UI_TONE_CURRENT);
    assert_row(&frame, 2u, "STATUS: CONNECTED");
    assert_row(&frame, 3u, "PROFILE: STANDARD");
}

static void test_disconnect_keeps_name_and_front_page(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_saved(&ux);
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(&ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(&ux, 1);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_current_mouse_name(&ux, NULL);
    blu2usb_ux_set_saved_connected_bond(&ux, -1);

    assert(ux.saved_front_bond == 1);
    assert(ux.saved_connected_bond == -1);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == 1);

    project(&ux, &frame);
    assert_row(&frame, 0u, "1 OF 3");
    assert_row(&frame, 1u, "LOGITECH LIFT MOUSE");
    assert_tone(&frame, 1u, BLU2USB_UI_TONE_STATIC);
    assert_row(&frame, 2u, "STATUS: DISCONNECTED");
}

static void test_another_connected_mouse_becomes_page_one(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_saved(&ux);

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(&ux, "Mouse Gamma");
    blu2usb_ux_set_saved_connected_bond(&ux, 2);

    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == 2);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 1u) == 0);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 2u) == 1);

    project(&ux, &frame);
    assert_row(&frame, 1u, "MOUSE GAMMA");
    assert_row(&frame, 2u, "STATUS: CONNECTED");

    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 1u);
    project(&ux, &frame);
    assert_row(&frame, 1u, "MOUSE ALPHA");
    assert_row(&frame, 2u, "STATUS: DISCONNECTED");

    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 2u);
    project(&ux, &frame);
    assert_row(&frame, 1u, "LOGITECH LIFT MOUSE");
    assert_row(&frame, 2u, "STATUS: DISCONNECTED");
}

static void test_navigation_access_back_and_lock_contract(void)
{
    blu2usb_ux_model_t ux;
    init_saved(&ux);
    blu2usb_ux_set_mouse_connected(false);

    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.saved_page == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 0u);

    const blu2usb_ux_command_t command =
        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(command.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);

    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
}

int main(void)
{
    test_connected_mouse_is_always_page_one();
    test_disconnect_keeps_name_and_front_page();
    test_another_connected_mouse_becomes_page_one();
    test_navigation_access_back_and_lock_contract();
    return 0;
}
