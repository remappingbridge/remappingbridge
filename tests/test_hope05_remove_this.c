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

static void init_two(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux, 1);
    ux->screen = BLU2USB_SCREEN_SAVED_DEVICES;
}

static void test_remove_this_projection_and_target(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_two(&ux);

    /* Connected logical bond 1 is page 0 under the accepted HOPE-04 order. */
    assert(blu2usb_ux_saved_bond_for_page(&ux, 0u) == 1);
    (void)tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(!ux.remove_pending);

    project(&ux, &frame);
    assert_row(&frame, 0u, "REMOVE THIS MOUSE");
    assert_row(&frame, 1u, "LOGITECH LIFT MOUSE");
    assert_row(&frame, 3u, "PAIRING AND MAPPINGS");
    assert_row(&frame, 4u, "WILL BE DELETED");
    assert_row(&frame, 6u, "KEY A: REMOVE");
    assert_row(&frame, 7u, "KEY B: CANCEL");
    assert_row(&frame, 8u, "KEY X: HELP");

    const blu2usb_ux_command_t remove =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(remove.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(remove.saved_bond == 1);
    assert(ux.remove_pending);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);

    /* Duplicate confirmation is consumed and cannot target the next Mouse. */
    const blu2usb_ux_command_t duplicate =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(duplicate.kind == BLU2USB_UX_COMMAND_NONE);
    assert(duplicate.saved_bond == -1);

    /* Once physical removal is pending, cancel cannot pretend it stopped it. */
    (void)tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
}

static void test_cancel_preserves_saved_page(void)
{
    blu2usb_ux_model_t ux;
    init_two(&ux);

    (void)tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 1u);
    assert(blu2usb_ux_saved_bond_for_page(&ux, 1u) == 0);

    (void)tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);

    (void)tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux.saved_page == 1u);
    assert(!ux.remove_pending);
}

static void test_remove_target_survives_connected_first_reorder(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_two(&ux);

    /* Browse to the disconnected Mouse Alpha (logical bond 0). */
    (void)tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 1u);
    assert(blu2usb_ux_saved_bond_for_page(&ux, ux.saved_page) == 0);

    (void)tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond == 0);

    /* Simulate background connection/order change while confirmation stays
     * open. The pinned removal identity and projected name must not change. */
    blu2usb_ux_set_saved_connected_bond(&ux, 0);
    assert(ux.saved_page == 0u);
    assert(ux.remove_target_bond == 0);

    project(&ux, &frame);
    assert_row(&frame, 1u, "MOUSE ALPHA");

    const blu2usb_ux_command_t remove =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(remove.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(remove.saved_bond == 0);
}


int main(void)
{
    test_remove_this_projection_and_target();
    test_cancel_preserves_saved_page();
    test_remove_target_survives_connected_first_reorder();
    return 0;
}
