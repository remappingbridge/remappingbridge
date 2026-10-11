/* MOD-04 corrective regression: four defects reported after physical flash.
 * Compile this test with assertions enabled even in CMake Release.
 */
#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    (void)blu2usb_ux_input(ux, key, true);
    return blu2usb_ux_input(ux, key, false);
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void expected_text(const blu2usb_ui_frame_t *frame,
                          unsigned row, const char *expected)
{
    char text[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned i=0u; i<BLU2USB_RENDERER_TEXT_COLS; ++i)
        text[i]=frame->cells[row][i].character;
    unsigned n=BLU2USB_RENDERER_TEXT_COLS;
    while (n && text[n-1u]==' ') --n;
    text[n]='\0';
    assert(strcmp(text, expected)==0);
}

static void expect_tone(const blu2usb_ui_frame_t *frame,
                        unsigned row, blu2usb_ui_tone_t expected)
{
    unsigned chars=0;
    for (unsigned col=0; col<BLU2USB_RENDERER_TEXT_COLS; ++col) {
        if (frame->cells[row][col].character==' ') continue;
        ++chars;
        assert(frame->cells[row][col].tone==expected);
    }
    assert(chars);
}

static const blu2usb_screen_id_t will_become[BLU2USB_MOUSE_SOURCE_COUNT]={
    BLU2USB_SCREEN_LEFT_WILL_BECOME,
    BLU2USB_SCREEN_RIGHT_WILL_BECOME,
    BLU2USB_SCREEN_MIDDLE_WILL_BECOME,
    BLU2USB_SCREEN_FORWARD_WILL_BECOME,
    BLU2USB_SCREEN_BACKWARD_WILL_BECOME
};
static const char *const source_names[BLU2USB_MOUSE_SOURCE_COUNT]={
    "LEFT", "RIGHT", "MIDDLE", "FORWARD", "BACKWARD"
};

static void test_modifier_summary_all_five_sources(void)
{
    for (unsigned src=0;src<BLU2USB_MOUSE_SOURCE_COUNT;++src) {
        blu2usb_ux_model_t ux;
        blu2usb_ui_frame_t frame;
        blu2usb_ux_init(&ux);
        ux.screen=will_become[src];
        ux.custom_source=(blu2usb_mouse_source_t)src;
        ux.selection=5u; /* MODIFIER last */
        const blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind==BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
        assert(cmd.target==BLU2USB_MOUSE_TARGET_MODIFIER);
        assert(ux.screen==will_become[src]); /* commit first */
        blu2usb_mouse_target_t committed[BLU2USB_MOUSE_SOURCE_COUNT]={
            BLU2USB_MOUSE_TARGET_LEFT,
            BLU2USB_MOUSE_TARGET_RIGHT,
            BLU2USB_MOUSE_TARGET_MIDDLE,
            BLU2USB_MOUSE_TARGET_FORWARD,
            BLU2USB_MOUSE_TARGET_BACKWARD
        };
        committed[src]=BLU2USB_MOUSE_TARGET_MODIFIER;
        blu2usb_ux_custom_target_committed(&ux,cmd.source,committed);
        assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
        project(&ux,&frame);
        char expected[BLU2USB_RENDERER_TEXT_COLS+1u];
        size_t n=0u;
        expected[n++]=' ';
        const char *source=source_names[src];
        while (*source) expected[n++]=*source++;
        for (const char *p=" IS MODIFIER";*p;++p) expected[n++]=*p;
        expected[n]='\0';
        expected_text(&frame,src+1u,expected);
        assert(frame.cells[src+1u][0].tone==BLU2USB_UI_TONE_EMPHASIZED);
    }
}

static void test_will_become_buttons_only_key_a_applies(void)
{
    for (unsigned src=0;src<BLU2USB_MOUSE_SOURCE_COUNT;++src) {
        blu2usb_ux_model_t ux;
        blu2usb_ux_init(&ux);
        ux.screen=will_become[src];
        ux.custom_source=(blu2usb_mouse_source_t)src;
        ux.selection=3u; /* FORWARD */

        for (unsigned i=0;i<3;++i) {
            const blu2usb_control_t key=i==0u ? BLU2USB_CONTROL_JOY_PRESS :
                (i==1u ? BLU2USB_CONTROL_KEY_X : BLU2USB_CONTROL_JOY_RIGHT);
            assert(tap(&ux,key).kind==BLU2USB_UX_COMMAND_NONE);
            assert(ux.screen==will_become[src]);
            assert(ux.selection==3u);
        }
        assert(ux.custom_targets[src] != BLU2USB_MOUSE_TARGET_MODIFIER);
        assert(tap(&ux,BLU2USB_CONTROL_KEY_B).kind==BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
        assert(ux.selection==src);

        ux.screen=will_become[src];
        ux.selection=5u;
        assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==
               BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
        assert(ux.screen==will_become[src]);
    }
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    ux.screen=BLU2USB_SCREEN_MIDDLE_WILL_BECOME;
    assert(tap(&ux,BLU2USB_CONTROL_KEY_Y).kind==BLU2USB_UX_COMMAND_NONE);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS); /* first unlock event, not an apply */
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_HOME);
}

static void test_applied_custom_both_hints_visible(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen=BLU2USB_SCREEN_EDIT_CUSTOM;
    ux.selection=2u;
    ux.custom_dirty=true;
    blu2usb_ux_profile_applied(&ux,BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP);
    assert(!ux.custom_dirty);
    project(&ux,&frame);
    expected_text(&frame,7u,"JOY PRESS: ACCESS");
    expected_text(&frame,8u,"KEY B: BACK");
    assert(tap(&ux,BLU2USB_CONTROL_KEY_B).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);

    blu2usb_ux_init(&ux);
    ux.screen=BLU2USB_SCREEN_EDIT_CUSTOM;
    ux.custom_dirty=true;
    project(&ux,&frame);
    expected_text(&frame,8u,"KEY A: APPLY CUSTOM");
}

static void test_single_cyan_only_for_current_target(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.custom_source=BLU2USB_MOUSE_SOURCE_MIDDLE;
    ux.screen=BLU2USB_SCREEN_MIDDLE_WILL_BECOME;
    ux.custom_targets[BLU2USB_MOUSE_SOURCE_MIDDLE]=BLU2USB_MOUSE_TARGET_FORWARD;
    ux.selection=0u;
    project(&ux,&frame);
    expect_tone(&frame,1u,BLU2USB_UI_TONE_EMPHASIZED);
    expect_tone(&frame,4u,BLU2USB_UI_TONE_CURRENT); /* FORWARD */
    expect_tone(&frame,5u,BLU2USB_UI_TONE_ACTIONABLE); /* BACKWARD */

    blu2usb_mouse_target_t committed[BLU2USB_MOUSE_SOURCE_COUNT];
    memcpy(committed,ux.custom_targets,sizeof(committed));
    committed[BLU2USB_MOUSE_SOURCE_MIDDLE]=BLU2USB_MOUSE_TARGET_BACKWARD;
    blu2usb_ux_custom_target_committed(&ux,BLU2USB_MOUSE_SOURCE_MIDDLE,committed);
    ux.screen=BLU2USB_SCREEN_MIDDLE_WILL_BECOME;
    ux.selection=0u;
    project(&ux,&frame);
    expect_tone(&frame,4u,BLU2USB_UI_TONE_ACTIONABLE); /* old FORWARD not cyan */
    expect_tone(&frame,5u,BLU2USB_UI_TONE_CURRENT); /* only BACKWARD cyan */
    expect_tone(&frame,6u,BLU2USB_UI_TONE_ACTIONABLE);

    ux.selection=4u; /* Selection overrides current with white. */
    project(&ux,&frame);
    expect_tone(&frame,4u,BLU2USB_UI_TONE_ACTIONABLE);
    expect_tone(&frame,5u,BLU2USB_UI_TONE_EMPHASIZED);

    committed[BLU2USB_MOUSE_SOURCE_MIDDLE]=BLU2USB_MOUSE_TARGET_MODIFIER;
    blu2usb_ux_custom_target_committed(&ux,BLU2USB_MOUSE_SOURCE_MIDDLE,committed);
    ux.screen=BLU2USB_SCREEN_MIDDLE_WILL_BECOME;
    ux.selection=0u;
    project(&ux,&frame);
    expect_tone(&frame,4u,BLU2USB_UI_TONE_ACTIONABLE);
    expect_tone(&frame,5u,BLU2USB_UI_TONE_ACTIONABLE);
    expect_tone(&frame,6u,BLU2USB_UI_TONE_CURRENT);
}
int main(void)
{
    test_modifier_summary_all_five_sources();
    test_will_become_buttons_only_key_a_applies();
    test_applied_custom_both_hints_visible();
    test_single_cyan_only_for_current_target();
    return 0;
}
