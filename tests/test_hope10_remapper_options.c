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

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux,
                                blu2usb_control_t control)
{
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}

static void init_remapper(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    ux->screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
}

static void assert_row(const blu2usb_ui_frame_t *frame,
                       unsigned row, const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    row_text(frame, row, actual);
    assert(strcmp(actual, expected) == 0);
}

static void assert_row_tone(const blu2usb_ui_frame_t *frame,
                            unsigned row, blu2usb_ui_tone_t tone)
{
    bool visible = false;
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
        if (frame->cells[row][col].character == ' ') continue;
        visible = true;
        assert(frame->cells[row][col].tone == tone);
    }
    assert(visible);
}

static void test_exact_layout(void)
{
    static const char *const expected[9] = {
        "REMAPPING OPTIONS",
        " PASSTHROUGH",
        " STANDARD REMAP",
        " ESCAPE REMAP",
        " CUSTOM REMAP",
        "",
        "JOY PRESS: ACCESS",
        "KEY B: BACK",
        "KEY X: HELP",
    };

    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_remapper(&ux);
    project(&ux, &frame);

    assert(blu2usb_ux_option_count(&ux) == 4u);
    assert(frame.hint_start_row == 6u);

    for (unsigned row = 0u; row < 9u; ++row)
        assert_row(&frame, row, expected[row]);

    const blu2usb_screen_template_t *screen =
        blu2usb_ux_screen_template(BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(screen != NULL);
    for (unsigned row = 0u; row < 9u; ++row) {
        const char *text = screen->rows[row] == NULL ? "" : screen->rows[row];
        assert(strstr(text, "PAIR MOUSE") == NULL);
        assert(strstr(text, "DEFAULT REMAP") == NULL);
    }
}

static void test_active_profile_and_selection_tones(void)
{
    static const struct {
        blu2usb_mouse_profile_kind_t profile;
        unsigned row;
    } cases[] = {
        {BLU2USB_MOUSE_PROFILE_PASSTHROUGH, 1u},
        {BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP, 2u},
        {BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP, 3u},
        {BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP, 4u},
    };

    for (unsigned i = 0u; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        init_remapper(&ux);
        ux.active_profile = cases[i].profile;
        ux.selection = (cases[i].row + 1u) % 4u;
        project(&ux, &frame);
        assert_row_tone(&frame, cases[i].row, BLU2USB_UI_TONE_CURRENT);

        ux.selection = cases[i].row - 1u;
        project(&ux, &frame);
        assert_row_tone(&frame, cases[i].row, BLU2USB_UI_TONE_EMPHASIZED);
    }
}

static void test_navigation(void)
{
    blu2usb_ux_model_t ux;

    init_remapper(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0u);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PASSTHROUGH_APPLIED);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    ux.selection = 0u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_PASSTHROUGH);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_DEFAULT_APPLIED);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_DEFAULT);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_ESCAPE_APPLIED);

    init_remapper(&ux);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_ESCAPE);

    init_remapper(&ux);
    ux.selection = 3u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
}

static void test_back_help_and_lock(void)
{
    blu2usb_ux_model_t ux;

    init_remapper(&ux);
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS);
    tap(&ux, BLU2USB_CONTROL_KEY_Y);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(ux.selection == 2u);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));

    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_HOME);

    init_remapper(&ux);
    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.screen == BLU2USB_SCREEN_HOME);

    init_remapper(&ux);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, true);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_PRESS, false);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME);
}


static void test_profile_change_cyan_is_exclusive(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_remapper(&ux);

    /* Passthrough is initially authoritative. Select Standard so the selected
     * row is white while only Passthrough is cyan. */
    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection = 1u;
    project(&ux, &frame);
    assert_row_tone(&frame, 1u, BLU2USB_UI_TONE_CURRENT);
    assert_row_tone(&frame, 2u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_row_tone(&frame, 3u, BLU2USB_UI_TONE_ACTIONABLE);
    assert_row_tone(&frame, 4u, BLU2USB_UI_TONE_ACTIONABLE);

    /* Simulate confirmed runtime+persistence Standard apply, return to options
     * and select an unrelated row. The old Passthrough row must lose cyan. */
    blu2usb_ux_profile_applied(&ux, BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    ux.selection = 3u;
    project(&ux, &frame);
    assert_row_tone(&frame, 1u, BLU2USB_UI_TONE_ACTIONABLE);
    assert_row_tone(&frame, 2u, BLU2USB_UI_TONE_CURRENT);
    assert_row_tone(&frame, 3u, BLU2USB_UI_TONE_ACTIONABLE);
    assert_row_tone(&frame, 4u, BLU2USB_UI_TONE_EMPHASIZED);

    /* Repeat once more to freeze against accumulating cyan rows. */
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_ESCAPE);
    (void)tap(&ux, BLU2USB_CONTROL_KEY_A);
    blu2usb_ux_profile_applied(&ux, BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    ux.selection = 0u;
    project(&ux, &frame);
    assert_row_tone(&frame, 1u, BLU2USB_UI_TONE_EMPHASIZED);
    assert_row_tone(&frame, 2u, BLU2USB_UI_TONE_ACTIONABLE);
    assert_row_tone(&frame, 3u, BLU2USB_UI_TONE_CURRENT);
    assert_row_tone(&frame, 4u, BLU2USB_UI_TONE_ACTIONABLE);
}

int main(void)
{
    test_exact_layout();
    test_active_profile_and_selection_tones();
    test_profile_change_cyan_is_exclusive();
    test_navigation();
    test_back_help_and_lock();
    return 0;
}
