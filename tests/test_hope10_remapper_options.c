#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t control) {
    (void)blu2usb_ux_input(ux, control, true);
    return blu2usb_ux_input(ux, control, false);
}
static void init(blu2usb_ux_model_t *ux) {
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
}
static void row(const blu2usb_ui_frame_t *f, unsigned n, const char *text) {
    char actual[BLU2USB_RENDERER_TEXT_COLS+1u];
    for (unsigned j=0; j<BLU2USB_RENDERER_TEXT_COLS; ++j)
        actual[j]=f->cells[n][j].character;
    unsigned len=BLU2USB_RENDERER_TEXT_COLS;
    while (len && actual[len-1]==' ') --len;
    actual[len]=0;
    assert(strcmp(actual,text)==0);
}
static void tone(const blu2usb_ui_frame_t *f, unsigned n, blu2usb_ui_tone_t t) {
    for(unsigned j=0;j<BLU2USB_RENDERER_TEXT_COLS;++j)
        if(f->cells[n][j].character!=' ') assert(f->cells[n][j].tone==t);
}
static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *f) {
    blu2usb_ui_project(ux,f);
    blu2usb_ui_enforce_applied_visual_contract(ux,f);
}
static void test_menu_and_help(void) {
    static const char *const expected[9]={
        "REMAPPING OPTIONS"," PASSTHROUGH DEFAULT"," CUSTOM REMAPPING",
        " MODIFIER SHORTCUTS","","JOY UP\\DOWN: SELECT",
        "JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"
    };
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t f;
    init(&ux);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS); /* HOME selected remapping */
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(blu2usb_ux_option_count(&ux)==3u);
    project(&ux,&f);
    for(unsigned n=0;n<9;++n) row(&f,n,expected[n]);
    assert(f.hint_start_row==5u);
    tap(&ux,BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection==2u);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection==0u);
    ux.selection=2u;
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert(ux.screen==BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS);
    project(&ux,&f);
    row(&f,0,"REMAPPING OPTIONS HELP");
    row(&f,5,"MODIFIER SHORTCUTS");
    tap(&ux,BLU2USB_CONTROL_KEY_Y); /* any key back, not lock */
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS && ux.selection==2u);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_HOME);
}
static void test_routes_and_no_legacy_commands(void) {
    blu2usb_ux_model_t ux;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.active_profile=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_PASSTHROUGH_APPLIED);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    ux.selection=1u;
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    ux.selection=2u;
    const blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(cmd.kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_1);
    /* Third menu item opens the combinations picker, not a legacy preset. */
}
static void test_colors_and_legacy_status(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t f;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.active_profile=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection=1u;
    project(&ux,&f);
    tone(&f,1u,BLU2USB_UI_TONE_CURRENT);
    tone(&f,2u,BLU2USB_UI_TONE_EMPHASIZED);
    tone(&f,3u,BLU2USB_UI_TONE_ACTIONABLE);
    ux.active_profile=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    ux.selection=0u;
    project(&ux,&f);
    tone(&f,1u,BLU2USB_UI_TONE_EMPHASIZED);
    tone(&f,2u,BLU2USB_UI_TONE_CURRENT);
    tone(&f,3u,BLU2USB_UI_TONE_ACTIONABLE);
    /* Legacy stored profiles are shown as PASSTHROUGH, not STANDARD/ESCAPE. */
    ux.active_profile=BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    ux.selection=2u;
    project(&ux,&f);
    tone(&f,3u,BLU2USB_UI_TONE_EMPHASIZED);
    ux.screen=BLU2USB_SCREEN_SAVED_DEVICES;
    ux.saved_page=0u;
    project(&ux,&f);
    row(&f,2,"PROFILE: PASSTHROUGH");
    ux.screen=BLU2USB_SCREEN_HOME;
    project(&ux,&f);
    row(&f,1," NO REMAP PASSTHROUGH");
}
static void test_lock_on_release(void) {
    blu2usb_ux_model_t ux;
    init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    (void)blu2usb_ux_input(&ux,BLU2USB_CONTROL_KEY_Y,true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux,BLU2USB_CONTROL_KEY_Y,false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_HOME);
}
int main(void) {
    test_menu_and_help();
    test_routes_and_no_legacy_commands();
    test_colors_and_legacy_status();
    test_lock_on_release();
    return 0;
}
