#include <assert.h>
#include <stdbool.h>
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

static void init_connected(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
}

static void assert_template(blu2usb_screen_id_t screen_id,
                            const char *const expected[9])
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_connected(&ux);
    ux.screen = screen_id;
    project(&ux, &frame);
    for (unsigned row = 0u; row < 9u; ++row)
        assert_row(&frame, row, expected[row]);
}

static void test_all_static_literals(void)
{
    static const char *const remapper[9] = {
        "REMAPPING OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP",
        " CUSTOM REMAP","","JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"
    };
    static const char *const help[9] = {
        "REMAPPER OPTIONS HELP","CHOOSE FROM THE","OPTIONS TO CHANGE THE",
        "FUNCTIONS OF THE","MOUSE BUTTONS.","PASSTHROUGH IS THE",
        "DEFAULT OPTION.","","ANY KEY: BACK"
    };
    static const char *const pass_active[9] = {
        "PASSTHROUGH ACTIVE","ORIGINAL MOUSE","BUTTONS POSITION","ARE ACTIVE NOW",
        "","","","KEY B: BACK","KEY Y: LOCK"
    };
    static const char *const pass_inactive[9] = {
        "APPLY PASSTHROUGH","ORIGINAL MOUSE","BUTTONS POSITION","ARE NOT ACTIVE",
        "","","KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"
    };
    static const char *const std_inactive[9] = {
        "APPLY STANDARD REMAP","FORWARD IS LEFT","LEFT IS FORWARD",
        "BACKWARD IS RIGHT","RIGHT IS BACKWARD","","KEY A: APPLY",
        "KEY B: CANCEL","KEY Y: LOCK"
    };
    static const char *const std_active[9] = {
        "STANDARD REMAP ACTIVE","FORWARD IS LEFT","LEFT IS FORWARD",
        "BACKWARD IS RIGHT","RIGHT IS BACKWARD","","","KEY B: BACK","KEY Y: LOCK"
    };
    static const char *const esc_inactive[9] = {
        "APPLY ESCAPE REMAP","FORWARD IS LEFT","BACKWARD IS RIGHT",
        "LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","",
        "KEY A: APPLY","KEY B: CANCEL"
    };
    static const char *const esc_active[9] = {
        "ESCAPE APPLIED ACTIVE","FORWARD IS LEFT","BACKWARD IS RIGHT",
        "LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD","",
        "KEY B: BACK","KEY Y: LOCK"
    };
    static const char *const custom_edit[9] = {
        "EDIT CUSTOM REMAP"," LEFT IS LEFT"," RIGHT IS RIGHT"," MIDDLE IS MIDDLE",
        " FORWARD IS FORWARD"," BACKWARD IS BACKWARD","","JOY PRESS: ACCESS",
        "KEY A: APPLY CUSTOM"
    };

    assert_template(BLU2USB_SCREEN_MOUSE_OPTIONS, remapper);
    assert_template(BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS, help);
    assert_template(BLU2USB_SCREEN_PASSTHROUGH_APPLIED, pass_active);
    assert_template(BLU2USB_SCREEN_APPLY_PASSTHROUGH, pass_inactive);
    assert_template(BLU2USB_SCREEN_APPLY_DEFAULT, std_inactive);
    assert_template(BLU2USB_SCREEN_DEFAULT_APPLIED, std_active);
    assert_template(BLU2USB_SCREEN_APPLY_ESCAPE, esc_inactive);
    assert_template(BLU2USB_SCREEN_ESCAPE_APPLIED, esc_active);
    assert_template(BLU2USB_SCREEN_EDIT_CUSTOM, custom_edit);
}

static void test_help_preserves_selection_and_y_is_back(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 2u;

    tap(&ux, BLU2USB_CONTROL_KEY_X);
    assert(ux.screen == BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS);
    assert(ux.return_selection == 2u);
    assert(blu2usb_ux_option_count(&ux) == 0u);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(ux.selection == 2u);
}

static void test_preset_active_inactive_and_commands(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);

    ux.active_profile = BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;

    ux.selection = 0u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_PASSTHROUGH);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind ==
           BLU2USB_UX_COMMAND_APPLY_PASSTHROUGH);

    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_APPLY_DEFAULT);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind ==
           BLU2USB_UX_COMMAND_APPLY_DEFAULT);

    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 2u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_ESCAPE_APPLIED);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);

    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection = 0u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_PASSTHROUGH_APPLIED);

    ux.active_profile = BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP;
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 1u;
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_DEFAULT_APPLIED);
}

static void test_active_screen_body_tones(void)
{
    static const blu2usb_screen_id_t active[] = {
        BLU2USB_SCREEN_PASSTHROUGH_APPLIED,
        BLU2USB_SCREEN_DEFAULT_APPLIED,
        BLU2USB_SCREEN_ESCAPE_APPLIED,
    };
    for (unsigned i = 0u; i < sizeof(active)/sizeof(active[0]); ++i) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        init_connected(&ux);
        ux.screen = active[i];
        project(&ux, &frame);
        for (unsigned row = 1u; row < frame.hint_start_row; ++row) {
            char text[BLU2USB_RENDERER_TEXT_COLS + 1u];
            row_text(&frame, row, text);
            if (text[0] != '\0')
                assert_row_tone(&frame, row, BLU2USB_UI_TONE_CURRENT);
        }
    }
}

static const blu2usb_screen_id_t editor_screen(unsigned source)
{
    static const blu2usb_screen_id_t screens[BLU2USB_MOUSE_SOURCE_COUNT] = {
        BLU2USB_SCREEN_LEFT_WILL_BECOME,
        BLU2USB_SCREEN_RIGHT_WILL_BECOME,
        BLU2USB_SCREEN_MIDDLE_WILL_BECOME,
        BLU2USB_SCREEN_FORWARD_WILL_BECOME,
        BLU2USB_SCREEN_BACKWARD_WILL_BECOME,
    };
    return screens[source];
}

static void test_source_editor_literals_and_mapping_order(void)
{
    static const char *const titles[BLU2USB_MOUSE_SOURCE_COUNT] = {
        "LEFT WILL BECOME","RIGHT WILL BECOME","MIDDLE WILL BECOME",
        "FORWARD WILL BECOME","BACKWARD WILL BECOME"
    };
    static const char *const targets[6] = {
        " LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD"
    };

    for (unsigned source = 0u; source < BLU2USB_MOUSE_SOURCE_COUNT; ++source) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        init_connected(&ux);
        ux.screen = editor_screen(source);
        ux.custom_source = (blu2usb_mouse_source_t)source;
        ux.selection = 0u;
        project(&ux, &frame);
        assert_row(&frame, 0u, titles[source]);
        for (unsigned row = 1u; row <= 6u; ++row)
            assert_row(&frame, row, targets[row - 1u]);
        assert_row(&frame, 8u, "KEY A: APPLY AND BACK");
    }

    /* UI row 4 means ESCAPE although the persisted/domain enum ordinal differs. */
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    ux.screen = BLU2USB_SCREEN_LEFT_WILL_BECOME;
    ux.custom_source = BLU2USB_MOUSE_SOURCE_LEFT;
    ux.selection = 3u;
    const blu2usb_ux_command_t escape =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(escape.kind == BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(escape.source == BLU2USB_MOUSE_SOURCE_LEFT);
    assert(escape.target == BLU2USB_MOUSE_TARGET_ESCAPE);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.selection == 0u);

    /* Current domain BACKWARD target must highlight visible row 6. */
    ux.screen = BLU2USB_SCREEN_LEFT_WILL_BECOME;
    ux.custom_source = BLU2USB_MOUSE_SOURCE_LEFT;
    ux.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT] =
        BLU2USB_MOUSE_TARGET_BACKWARD;
    ux.selection = 0u;
    blu2usb_ui_frame_t frame;
    project(&ux, &frame);
    assert_row_tone(&frame, 6u, BLU2USB_UI_TONE_CURRENT);
    assert_row_tone(&frame, 1u, BLU2USB_UI_TONE_EMPHASIZED);
}

static void test_editor_back_and_joy_press_return_to_source_row(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    ux.screen = BLU2USB_SCREEN_EDIT_CUSTOM;
    ux.selection = 3u;

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_FORWARD_WILL_BECOME);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.selection == 3u);

    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_FORWARD_WILL_BECOME);
    ux.selection = 5u;
    const blu2usb_ux_command_t cmd =
        tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(cmd.kind == BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(cmd.source == BLU2USB_MOUSE_SOURCE_FORWARD);
    assert(cmd.target == BLU2USB_MOUSE_TARGET_BACKWARD);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.selection == 3u);
}

static void test_custom_apply_success_stays_edit(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init_connected(&ux);
    ux.screen = BLU2USB_SCREEN_EDIT_CUSTOM;
    ux.selection = 2u;
    ux.custom_dirty = true;

    const blu2usb_ux_command_t request =
        tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(request.kind == BLU2USB_UX_COMMAND_APPLY_CUSTOM);

    blu2usb_ux_profile_applied(&ux, BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.selection == 2u);
    assert(!ux.custom_dirty);

    project(&ux, &frame);
    for (unsigned row = 1u; row <= 5u; ++row) {
        assert_row_tone(
            &frame, row,
            row == 3u ? BLU2USB_UI_TONE_EMPHASIZED :
                        BLU2USB_UI_TONE_CURRENT);
    }
}

static void test_global_lock_where_not_help(void)
{
    const blu2usb_screen_id_t screens[] = {
        BLU2USB_SCREEN_APPLY_ESCAPE,
        BLU2USB_SCREEN_EDIT_CUSTOM,
        BLU2USB_SCREEN_LEFT_WILL_BECOME,
    };
    for (unsigned i = 0u; i < sizeof(screens)/sizeof(screens[0]); ++i) {
        blu2usb_ux_model_t ux;
        init_connected(&ux);
        ux.screen = screens[i];
        (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
        (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
        assert(blu2usb_interaction_is_locked(&ux.interaction));
    }
}

int main(void)
{
    test_all_static_literals();
    test_help_preserves_selection_and_y_is_back();
    test_preset_active_inactive_and_commands();
    test_active_screen_body_tones();
    test_source_editor_literals_and_mapping_order();
    test_editor_back_and_joy_press_return_to_source_row();
    test_custom_apply_success_stays_edit();
    test_global_lock_where_not_help();
    return 0;
}
