#include <assert.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux,
                                blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}

static void assert_no_learn_option(blu2usb_screen_id_t screen_id,
                                   unsigned expected_count)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = screen_id;

    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(screen_id);
    assert(screen != NULL);
    assert(blu2usb_ux_option_count(&ux) == expected_count);

    for (unsigned row = 0u; row < 9u; ++row) {
        const char *text = screen->rows[row] == NULL ? "" : screen->rows[row];
        assert(strstr(text, "LEARN THE KEYS") == NULL);
    }
}

static void test_home_connected_has_three_routes(void)
{
    blu2usb_ux_model_t ux;

    assert_no_learn_option(BLU2USB_SCREEN_HOME, 3u);

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME;

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    ux.selection = 0u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);

    ux.screen = BLU2USB_SCREEN_HOME;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    ux.screen = BLU2USB_SCREEN_HOME;
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
}

static void test_home_searching_has_two_routes(void)
{
    blu2usb_ux_model_t ux;

    assert_no_learn_option(BLU2USB_SCREEN_HOME_SEARCHING, 2u);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    ux.screen = BLU2USB_SCREEN_HOME_SEARCHING;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
}

static void test_home_retry_has_two_routes(void)
{
    blu2usb_ux_model_t ux;

    assert_no_learn_option(BLU2USB_SCREEN_HOME_RETRY, 2u);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME_RETRY;

    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    ux.screen = BLU2USB_SCREEN_HOME_RETRY;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);
}

static void test_first_mouse_automatic_search_is_preserved(void)
{
    blu2usb_ux_model_t ux;

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 0u);

    /* Internal legacy enum still represents the accepted searching-first
     * screen. It is automatic only; no HOME route may expose it. */
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_SEARCHING_FIRST);

    const blu2usb_screen_template_t *searching =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_SEARCHING_FIRST);
    assert(searching != NULL);
    assert(strcmp(searching->rows[0], "SEARCHING FIRST MOUSE") == 0);
}

static void test_home_help_clamps_to_three_options(void)
{
    blu2usb_ux_model_t ux;

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_HOME;
    ux.selection = 2u;

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 2u);
}

int main(void)
{
    test_home_connected_has_three_routes();
    test_home_searching_has_two_routes();
    test_home_retry_has_two_routes();
    test_first_mouse_automatic_search_is_preserved();
    test_home_help_clamps_to_three_options();
    return 0;
}
