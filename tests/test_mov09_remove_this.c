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

static void frame_for(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expect_row(const blu2usb_ui_frame_t *frame,
                       unsigned row, const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    for (unsigned i = 0; i < n; ++i)
        actual[i] = frame->cells[row][i].character;
    while (n > 0 && actual[n - 1] == ' ') --n;
    actual[n] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void init_three(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 3u);
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 2u, "Mouse Gamma");
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux, 1);
    blu2usb_ux_set_saved_movement(ux, 0u,
        (blu2usb_movement_settings_t){2u, 12u, 1u});
    blu2usb_ux_set_saved_movement(ux, 2u,
        (blu2usb_movement_settings_t){5u, 6u, 2u});
    blu2usb_ux_set_movement(ux,
        (blu2usb_movement_settings_t){8u, 3u, 3u});
    ux->screen = BLU2USB_SCREEN_HOME;
}

static void open_saved(blu2usb_ux_model_t *ux)
{
    assert(ux->screen == BLU2USB_SCREEN_HOME);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection == 2u);
    tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_SAVED_DEVICES);
}

static void open_remove_on_page(blu2usb_ux_model_t *ux,
                                unsigned page, int logical_bond)
{
    while (ux->saved_page != page)
        tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(blu2usb_ux_saved_bond_for_page(ux, page) == logical_bond);
    assert(!ux->remove_pending);
    const blu2usb_ux_command_t access = tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(access.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux->screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux->remove_target_bond == logical_bond);
}

static void test_cancel_preserves_logical_target_page_and_values(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_three(&ux);
    open_saved(&ux);

    /* Connected bond 1 appears on first page, but the selected inactive
       bond 0 on second page must be the target, not page ordinal 1. */
    open_remove_on_page(&ux, 1u, 0);
    frame_for(&ux, &frame);
    static const char *const rows[9] = {
        "REMOVE THIS MOUSE",
        "MOUSE ALPHA",
        "SAVED PAIRING AND",
        "PREFERENCES WILL",
        "BE DELETED.",
        "",
        "KEY A: REMOVE",
        "KEY B: CANCEL",
        "KEY Y: HELP"
    };
    for (unsigned i = 0; i < 9; ++i)
        expect_row(&frame, i, rows[i]);
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_STATIC);
    assert(ux.saved_movements[0].horizontal_quarters == 2u);
    assert(ux.saved_movements[1].horizontal_quarters == 8u);
    assert(ux.saved_movements[2].horizontal_quarters == 5u);

    /* Help is not a global lock; any key returns to this exact bond. */
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMOVE_THIS);
    frame_for(&ux, &frame);
    expect_row(&frame, 0u, "REMOVE MOUSE HELP");
    expect_row(&frame, 6u, "CUSTOM MOVEMENTS");
    expect_row(&frame, 8u, "ANY KEY: BACK");
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.remove_target_bond == 0);

    const blu2usb_ux_command_t cancel = tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(cancel.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux.saved_page == 1u);
    assert(ux.remove_target_bond == -1);
    assert(!ux.remove_pending);
    assert(ux.saved_device_count == 3u);
    assert(ux.saved_movements[0].vertical_quarters == 12u);
    assert(ux.saved_movements[1].rotation_quadrants == 3u);
    assert(ux.saved_movements[2].rotation_quadrants == 2u);
}

static void test_single_command_pending_prevents_second_deletion(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_three(&ux);
    open_saved(&ux);
    open_remove_on_page(&ux, 2u, 2);
    frame_for(&ux, &frame);
    expect_row(&frame, 1u, "MOUSE GAMMA");

    /* Mouse removal is only REQUESTED on Key A RELEASE, never on press. */
    const blu2usb_ux_command_t down =
        blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    assert(!ux.remove_pending);
    const blu2usb_ux_command_t cmd =
        blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_A, false);
    assert(cmd.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(cmd.saved_bond == 2); /* logical bond, not page number */
    assert(ux.remove_pending);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.saved_device_count == 3u); /* no premature UI deletion */
    assert(ux.saved_movements[2].vertical_quarters == 6u);

    for (unsigned i = 0u; i < 3u; ++i) {
        const blu2usb_ux_command_t repeat = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(repeat.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.remove_target_bond == 2);
    }
    assert(tap(&ux, BLU2USB_CONTROL_KEY_B).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_Y).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_REMOVE_THIS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind ==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.remove_pending && ux.saved_device_count == 3u);

    /* Simulate main.c BLE REMOVE_FAILED: no bond deleted; return to
       inventory and make the chosen mouse retryable. */
    ux.remove_pending = false;
    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    ux.selection = 0u;
    assert(blu2usb_ux_saved_bond_for_page(&ux, ux.saved_page) == 2);
    assert(ux.saved_movements[2].vertical_quarters == 6u);
    open_remove_on_page(&ux, 2u, 2);
    const blu2usb_ux_command_t retry = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(retry.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(retry.saved_bond == 2);
}

static void test_active_identity_and_empty_registry_transition(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_three(&ux);
    open_saved(&ux);
    open_remove_on_page(&ux, 0u, 1);
    frame_for(&ux, &frame);
    expect_row(&frame, 1u, "LOGITECH LIFT MOUSE");
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_CURRENT);
    const blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind == BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(cmd.saved_bond == 1);
    assert(ux.saved_device_count == 3u);
    assert(ux.saved_connected_bond == 1);

    /* Pico event REMOVE_SUCCEEDED must clear current identity and then
       synchronize the remaining bonds. The UI simulation models its two
       safe destinations; it does not claim to delete physical BLE bonds. */
    ux.remove_pending = false;
    ux.remove_target_bond = -1;
    ux.saved_front_bond = -1;
    ux.saved_connected_bond = -1;
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_saved_device_count(&ux, 2u);
    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    assert(ux.saved_device_count == 2u);
    assert(ux.saved_pages == 2u);
    assert(ux.remove_target_bond == -1);
    assert(!ux.remove_pending);
    /* After last remaining bond is removed, bootstrap first mouse. */
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.screen = BLU2USB_SCREEN_SEARCHING_FIRST;
    ux.saved_page = 0u;
    assert(ux.saved_device_count == 0u);
    assert(ux.screen == BLU2USB_SCREEN_SEARCHING_FIRST);
}

int main(void)
{
    test_cancel_preserves_logical_target_page_and_values();
    test_single_command_pending_prevents_second_deletion();
    test_active_identity_and_empty_registry_transition();
    return 0;
}
