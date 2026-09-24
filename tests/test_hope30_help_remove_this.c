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

static void init_remove_this(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux, 1);
    ux->screen = BLU2USB_SCREEN_SAVED_DEVICES;

    (void)tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux->saved_page == 1u);
    assert(blu2usb_ux_saved_bond_for_page(ux, ux->saved_page) == 0);
    (void)tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux->remove_target_bond == 0);
}

static void test_help_layout(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_remove_this(&ux);

    (void)tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMOVE_THIS);
    assert(ux.return_screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);

    blu2usb_ui_project(&ux, &frame);
    assert_row(&frame, 0u, "REMOVE MOUSE HELP");
    assert_row(&frame, 1u, "COMPLETELY REMOVE THE");
    assert_row(&frame, 2u, "AUTOMATIC CONNECTION");
    assert_row(&frame, 3u, "WHEN TURNING ON THE");
    assert_row(&frame, 4u, "DEVICE AND DELETE ITS");
    assert_row(&frame, 5u, "BUTTON REMAPPING");
    assert_row(&frame, 6u, "PROFILE.");
    assert_row(&frame, 7u, "");
    assert_row(&frame, 8u, "ANY KEY: BACK");
}

static void test_any_key_returns_without_locking(void)
{
    const blu2usb_control_t controls[] = {
        BLU2USB_CONTROL_JOY_UP,
        BLU2USB_CONTROL_JOY_DOWN,
        BLU2USB_CONTROL_JOY_LEFT,
        BLU2USB_CONTROL_JOY_RIGHT,
        BLU2USB_CONTROL_JOY_PRESS,
        BLU2USB_CONTROL_KEY_A,
        BLU2USB_CONTROL_KEY_B,
        BLU2USB_CONTROL_KEY_X,
        BLU2USB_CONTROL_KEY_Y,
    };

    for (unsigned i = 0u; i < sizeof(controls) / sizeof(controls[0]); ++i) {
        blu2usb_ux_model_t ux;
        init_remove_this(&ux);
        (void)tap(&ux, BLU2USB_CONTROL_KEY_X);
        assert(ux.screen == BLU2USB_SCREEN_HELP_REMOVE_THIS);

        const int target = ux.remove_target_bond;
        const unsigned page = ux.saved_page;
        (void)tap(&ux, controls[i]);

        assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
        assert(ux.remove_target_bond == target);
        assert(ux.saved_page == page);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
    }
}

static void test_help_preserves_pinned_target_across_reorder(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_remove_this(&ux);
    (void)tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMOVE_THIS);

    /* A background reconnect may reorder Saved Devices while Help is open. */
    blu2usb_ux_set_saved_connected_bond(&ux, 0);
    assert(ux.saved_page == 0u);
    assert(ux.remove_target_bond == 0);

    (void)tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    blu2usb_ui_project(&ux, &frame);
    assert_row(&frame, 1u, "MOUSE ALPHA");

    const blu2usb_ux_command_t remove =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(remove.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(remove.saved_bond == 0);
}

static void test_help_not_entered_after_remove_started(void)
{
    blu2usb_ux_model_t ux;
    init_remove_this(&ux);

    const blu2usb_ux_command_t remove =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(remove.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(ux.remove_pending);

    (void)tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
}

int main(void)
{
    test_help_layout();
    test_any_key_returns_without_locking();
    test_help_preserves_pinned_target_across_reorder();
    test_help_not_entered_after_remove_started();
    return 0;
}
