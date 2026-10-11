#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "blu2usb/movement/movement.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t press = blu2usb_ux_input(ux, key, true);
    assert(press.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux, key, false);
}

static void draw(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expect_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned col = 0; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        actual[col] = frame->cells[row][col].character;
    unsigned n = BLU2USB_RENDERER_TEXT_COLS;
    while (n > 0u && actual[n - 1u] == ' ') --n;
    actual[n] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void init_connected(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    blu2usb_ux_set_current_mouse_name(ux, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 0u, "Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux, 1u, "Mouse Alpha");
    blu2usb_ux_set_saved_connected_bond(ux, 0);
    ux->active_profile = BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP;
    blu2usb_ux_set_movement(ux, (blu2usb_movement_settings_t){8u, 3u, 3u});
    ux->screen = BLU2USB_SCREEN_HOME;
}

static void test_four_options_and_exact_connected_home(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_connected(&ux);
    assert(blu2usb_ux_option_count(&ux) == 4u);
    draw(&ux, &frame);
    assert(frame.hint_start_row == 6u);
    static const char *const rows[9] = {
        "LOGITECH LIFT MOUSE",
        " NO REMAP PASSTHROUGH",
        " MOVEMENT OPTIONS",
        " SAVED DEVICES",
        " PAIR NEW MOUSE",
        "",
        "JOY UP / DOWN: SELECT",
        "JOY PRESS: ACCESS",
        "KEY X: HELP TO REMOVE"
    };
    for (unsigned row = 0; row < 9; ++row)
        expect_row(&frame, row, rows[row]);

    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_ACTIONABLE);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, true);
    assert(ux.selection == 0u); /* only release navigates */
    draw(&ux, &frame);
    assert(frame.cells[7][0].tone == BLU2USB_UI_TONE_ACTIONABLE);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, false);
    assert(ux.selection == 1u);
    draw(&ux, &frame);
    assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_ACTIONABLE);

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u); /* wrap */
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);
    tap(&ux, BLU2USB_CONTROL_KEY_B); /* v1 HOME has no Back */
    assert(ux.screen == BLU2USB_SCREEN_HOME && ux.selection == 0u);
}

static void test_all_home_routes_and_preserved_preferences(void)
{
    static const blu2usb_screen_id_t targets[4] = {
        BLU2USB_SCREEN_MOUSE_OPTIONS,
        BLU2USB_SCREEN_MOVEMENT_OPTIONS,
        BLU2USB_SCREEN_SAVED_DEVICES,
        BLU2USB_SCREEN_PAIR_MOUSE
    };
    for (unsigned option = 0; option < 4u; ++option) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        init_connected(&ux);
        for (unsigned i = 0; i < option; ++i)
            tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        assert(ux.selection == option);
        const blu2usb_ux_command_t entered = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
        assert(entered.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == targets[option]);

        if (option == 1u) {
            draw(&ux, &frame);
            expect_row(&frame, 1u, " HORIZONTAL DPI 2");
            expect_row(&frame, 2u, " VERTICAL DPI 0.75");
            expect_row(&frame, 4u, " ROTATION 270-DEGREE");
        }
        if (option == 2u) {
            assert(ux.saved_page == 0u);
            draw(&ux, &frame);
            expect_row(&frame, 1u, "LOGITECH LIFT MOUSE");
            expect_row(&frame, 3u, "DPI: H 2 - V 0.75");
            expect_row(&frame, 4u, "ROTATION: 270 DEGREE");
            /* The active per-bond snapshot must agree with the connected
               movement preferences already acknowledged by the UI. */
        }
        const blu2usb_ux_command_t back = tap(&ux, BLU2USB_CONTROL_KEY_B);
        assert(back.kind == BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen == BLU2USB_SCREEN_HOME);
        assert(ux.selection == 0u);
        assert(ux.movement_current.horizontal_quarters == 8u);
        assert(ux.movement_current.vertical_quarters == 3u);
        assert(ux.movement_current.rotation_quadrants == 3u);
        assert(ux.active_profile == BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP);
    }
}

static void test_home_help_and_return_selection(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_connected(&ux);
    for (unsigned i = 0u; i < 3u; ++i)
        tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);
    draw(&ux, &frame);
    expect_row(&frame, 0u, "REMOVE CONNECTED HELP");
    expect_row(&frame, 4u, "STEP 1. SAVED DEVICES");
    expect_row(&frame, 8u, "ANY KEY: BACK");

    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 3u); /* selection 4 of 4 restored, not modulo 3 */
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.movement_current.rotation_quadrants == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
}

static void test_lock_unlock_consumes_key_and_retains_movement(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    const blu2usb_ux_command_t unlock = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(unlock.kind == BLU2USB_UX_COMMAND_NONE);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 0u);
    assert(ux.movement_current.horizontal_quarters == 8u);
    assert(ux.movement_current.vertical_quarters == 3u);
    assert(ux.movement_current.rotation_quadrants == 3u);
    assert(ux.active_profile == BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP);
    /* The unlocking tap must NOT enter a menu or change settings. */
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
}

static void test_disconnected_search_retry_and_reconnect_route(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_connected(&ux);
    const blu2usb_movement_settings_t persisted = ux.movement_current;
    /* Simulate the saved-mouse disconnection transition dispatched by
       service_ble_messages in main.c, and the UX state shown by it. */
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_saved_connected_bond(&ux, -1);
    blu2usb_ux_set_current_mouse_name(&ux, NULL);
    blu2usb_ux_set_movement(&ux, blu2usb_movement_defaults());
    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;
    ux.selection = 0u;
    assert(blu2usb_ux_option_count(&ux) == 2u);
    draw(&ux, &frame);
    expect_row(&frame, 0u, "SEARCHING SAVED MOUSE");
    expect_row(&frame, 1u, " SAVED DEVICES");
    expect_row(&frame, 2u, " PAIR NEW MOUSE");
    for (unsigned row = 0u; row < 9u; ++row) {
        char buf[BLU2USB_RENDERER_TEXT_COLS + 1u];
        for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
            buf[col] = frame.cells[row][col].character;
        buf[BLU2USB_RENDERER_TEXT_COLS] = '\0';
        assert(strstr(buf, "MOVEMENT") == NULL);
    }
    tap(&ux, BLU2USB_CONTROL_KEY_B); /* cancel search -> RETRY */
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);
    draw(&ux, &frame);
    expect_row(&frame, 0u, "DEVICE NOT FOUND");
    assert(blu2usb_ux_option_count(&ux) == 2u);
    expect_row(&frame, 1u, " SAVED DEVICES");
    expect_row(&frame, 2u, " PAIR NEW MOUSE");
    tap(&ux, BLU2USB_CONTROL_KEY_B); /* existing Retry Key B is no-op */
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING_HELP);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.screen == BLU2USB_SCREEN_HOME_RETRY);

    /* Simulate BLE CONNECTED: main.c re-loads persisted movement from
       per-bond TLV before showing connected HOME. */
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_set_current_mouse_name(&ux, "Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(&ux, 0);
    blu2usb_ux_set_movement(&ux, persisted);
    ux.screen = BLU2USB_SCREEN_HOME;
    ux.selection = 0u;
    assert(blu2usb_ux_option_count(&ux) == 4u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    draw(&ux, &frame);
    expect_row(&frame, 1u, " HORIZONTAL DPI 2");
    expect_row(&frame, 2u, " VERTICAL DPI 0.75");
    expect_row(&frame, 4u, " ROTATION 270-DEGREE");
}

int main(void)
{
    test_four_options_and_exact_connected_home();
    test_all_home_routes_and_preserved_preferences();
    test_home_help_and_return_selection();
    test_lock_unlock_consumes_key_and_retains_movement();
    test_disconnected_search_retry_and_reconnect_route();
    return 0;
}
