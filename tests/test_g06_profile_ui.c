/* G06 physical UX regressions adapted by MOD-01.
 * Keep passthrough/Custom behavior and current-value renderer checks.
 * Retired Standard/Escape presets are no longer interactive choices. */
#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t c) {
    (void)blu2usb_ux_input(ux,c,true);
    return blu2usb_ux_input(ux,c,false);
}
static void init(blu2usb_ux_model_t *ux) {
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux,1u);
}
static void project(blu2usb_ux_model_t *ux,blu2usb_ui_frame_t *f) {
    blu2usb_ui_project(ux,f);
    blu2usb_ui_enforce_applied_visual_contract(ux,f);
}
static void tone(blu2usb_ui_frame_t *f,unsigned row,blu2usb_ui_tone_t expected) {
    for(unsigned j=0;j<BLU2USB_RENDERER_TEXT_COLS;++j)
        if(f->cells[row][j].character!=' ') assert(f->cells[row][j].tone==expected);
}
static void row_text(blu2usb_ui_frame_t *f,unsigned row,const char *expected) {
    char s[BLU2USB_RENDERER_TEXT_COLS+1];
    for(unsigned j=0;j<BLU2USB_RENDERER_TEXT_COLS;++j)s[j]=f->cells[row][j].character;
    unsigned end=BLU2USB_RENDERER_TEXT_COLS;
    while(end&&s[end-1]==' ')--end;
    s[end]=0;
    assert(strcmp(s,expected)==0);
}
static void test_passthrough_feedback(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t f;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection=0u;
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_PASSTHROUGH_APPLIED);
    project(&ux,&f);
    row_text(&f,0,"PASSTHROUGH ACTIVE");
    tone(&f,1u,BLU2USB_UI_TONE_CURRENT);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
}
static void test_custom_edit_and_apply(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t f;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection=1u;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.custom_dirty);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_LEFT_WILL_BECOME);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    const blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(cmd.source==BLU2USB_MOUSE_SOURCE_LEFT);
    assert(cmd.target==BLU2USB_MOUSE_TARGET_RIGHT);
    blu2usb_ux_set_custom_target(&ux,cmd.source,cmd.target);
    project(&ux,&f);
    row_text(&f,1u," LEFT IS RIGHT");
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_APPLY_CUSTOM);
    blu2usb_ux_profile_applied(&ux,BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP);
    assert(!ux.custom_dirty);
    project(&ux,&f);
    tone(&f,1u,BLU2USB_UI_TONE_EMPHASIZED);
    tone(&f,2u,BLU2USB_UI_TONE_CURRENT);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    ux.selection=0u;
    project(&ux,&f);
    tone(&f,1u,BLU2USB_UI_TONE_EMPHASIZED);
    tone(&f,2u,BLU2USB_UI_TONE_CURRENT);
}
static void test_third_choice_does_not_activate_legacy_presets(void) {
    blu2usb_ux_model_t ux;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection=2u;
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(ux.selection==2u);
}
int main(void) {
    test_passthrough_feedback();
    test_custom_edit_and_apply();
    test_third_choice_does_not_activate_legacy_presets();
    return 0;
}
