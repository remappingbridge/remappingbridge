#include <assert.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char out[BLU2USB_RENDERER_TEXT_COLS + 1u])
{
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        out[col] = frame->cells[row][col].character;
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    while (end > 0u && out[end - 1u] == ' ') --end;
    out[end] = '\0';
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void assert_row(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    row_text(frame, row, actual);
    assert(strcmp(actual, expected) == 0);
}

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux,
                                blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}

static void init_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_HOME;
}

static void test_static_layout_and_no_legacy_home(void)
{
    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_HOME);
    assert(screen != NULL);

    assert(strcmp(screen->rows[2], " SAVED DEVICES") == 0);
    assert(strcmp(screen->rows[3], " PAIR NEW MOUSE") == 0);
    assert(strcmp(screen->rows[4], " LEARN THE KEYS") == 0);
    assert(strcmp(screen->rows[6], "JOY UP / DOWN: SELECT") == 0);
    assert(strcmp(screen->rows[7], "JOY PRESS: ACCESS") == 0);
    assert(strcmp(screen->rows[8], "KEY X: HELP TO REMOVE") == 0);

    for (unsigned row = 0u; row < 9u; ++row) {
        const char *text = screen->rows[row] == NULL ? "" : screen->rows[row];
        assert(strcmp(text, "HOME") != 0);
        assert(strcmp(text, " STATUS") != 0);
        assert(strcmp(text, " OTHER OPTIONS") != 0);
        assert(strcmp(text, "KEY Y: LOCK / UNLOCK") != 0);
    }
}

static void assert_home_title(const char *input, const char *expected)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_home(&ux);
    blu2usb_ux_set_current_mouse_name(&ux, input);
    project(&ux, &frame);
    assert_row(&frame, 0u, expected);
}

static void test_home_title_rules(void)
{
    assert_home_title("LIFT", "LIFT MOUSE");
    assert_home_title("mouse generic", "MOUSE GENERIC");
    assert_home_title("XPTO ULTRA 2714", "XPTO ULTRA 2714 MOUSE");
    assert_home_title("ABCDEFGHIJKLMNOP", "ABCDEFGHIJKLMNO MOUSE");
    assert_home_title("ABCDEFGHIJKLMNO MOUSE", "ABCDEFGHIJKLMNO");
    assert_home_title("MOUSEPAD", "MOUSEPAD MOUSE");
    assert_home_title("***", "UNKNOWN MOUSE");
    assert_home_title("", "UNKNOWN MOUSE");
    assert_home_title(NULL, "UNKNOWN MOUSE");
}

static void test_profile_summaries_and_tones(void)
{
    static const struct {
        blu2usb_mouse_profile_kind_t profile;
        const char *summary;
    } cases[] = {
        {BLU2USB_MOUSE_PROFILE_PASSTHROUGH, " NO REMAP PASSTHROUGH"},
        {BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP, " REMAPPED TO STANDARD"},
        {BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP, " REMAPPED TO ESCAPE"},
        {BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP, " REMAPPED TO CUSTOM"},
    };

    for (unsigned i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        init_home(&ux);
        blu2usb_ux_set_current_mouse_name(&ux, "LOGITECH LIFT");
        blu2usb_ux_profile_applied(&ux, cases[i].profile);
        ux.selection = 1u;
        project(&ux, &frame);

        assert_row(&frame, 0u, "LOGITECH LIFT MOUSE");
        assert_row(&frame, 1u, cases[i].summary);
        assert(frame.hint_start_row == 6u);
        assert(blu2usb_renderer_background_rgb565(&frame, 5u) ==
               BLU2USB_COLOR_BLACK);
        assert(blu2usb_renderer_background_rgb565(&frame, 6u) ==
               BLU2USB_COLOR_DARK_MAGENTA);

        /* Summary is an ordinary action row when it is not selected. */
        assert(frame.cells[1][1].tone == BLU2USB_UI_TONE_ACTIONABLE);
        /* Selected Saved Devices row is white. */
        assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
        assert(frame.cells[1][1].tone != BLU2USB_UI_TONE_CURRENT);
        assert(frame.cells[2][1].tone != BLU2USB_UI_TONE_CURRENT);
    }
}

static void test_navigation_and_future_help(void)
{
    blu2usb_ux_model_t ux;
    init_home(&ux);

    assert(blu2usb_ux_option_count(&ux) == 4u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    blu2usb_ux_command_t command = tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(command.kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);

    init_home(&ux);
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_SAVED_DEVICES);

    init_home(&ux);
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PAIR_MOUSE);

    init_home(&ux);
    ux.selection = 3u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);

    init_home(&ux);
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED);
    tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 2u);

    init_home(&ux);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, true);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, false);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 0u);
}

static void test_home_resolver(void)
{
    blu2usb_ux_model_t ux;

    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);

    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 1u);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME_SEARCHING);

    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_LEARN_KEYS);
}

static void test_hope27_title_improvement(void)
{
    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);
    assert(screen != NULL);
    assert(strcmp(screen->rows[0], "MOUSE NOT FOUND HELP") == 0);
    assert(strcmp(screen->rows[8], "ANY KEY: BACK") == 0);
}

int main(void)
{
    test_static_layout_and_no_legacy_home();
    test_home_title_rules();
    test_profile_summaries_and_tones();
    test_navigation_and_future_help();
    test_home_resolver();
    test_hope27_title_improvement();
    return 0;
}
