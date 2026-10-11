#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t c) {
    (void)blu2usb_ux_input(ux,c,true);
    (void)blu2usb_ux_input(ux,c,false);
}
static blu2usb_ux_command_t invoke(blu2usb_ux_model_t *ux, blu2usb_control_t c) {
    (void)blu2usb_ux_input(ux,c,true);
    return blu2usb_ux_input(ux,c,false);
}
static void init(blu2usb_ux_model_t *ux) {
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux,1u);
}
static void expect_screen_rows(blu2usb_screen_id_t id, const char* const rows[9]) {
    const blu2usb_screen_template_t *s=blu2usb_ux_screen_template(id);
    assert(s);
    for (unsigned i=0;i<9;++i) assert(strcmp(s->rows[i],rows[i])==0);
}
static void test_replacement_menu_and_help(void) {
    static const char* const menu[9]={
        "REMAPPING OPTIONS"," PASSTHROUGH DEFAULT"," CUSTOM REMAPPING",
        " MODIFIER SHORTCUTS","","JOY UP\\DOWN: SELECT","JOY PRESS: ACCESS",
        "KEY B: BACK","KEY X: HELP"
    };
    static const char* const help[9]={
        "REMAPPING OPTIONS HELP","PASSTHROUGH DEFAULT","KEEPS ORIGINAL BUTTONS",
        "CUSTOM REMAPPING","EDITS MOUSE BUTTONS","MODIFIER SHORTCUTS",
        "CONFIGURES SHORTCUTS","","ANY KEY: BACK"
    };
    expect_screen_rows(BLU2USB_SCREEN_MOUSE_OPTIONS,menu);
    expect_screen_rows(BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS,help);
}
static void test_connected_route_and_persistence_command_boundary(void) {
    blu2usb_ux_model_t ux;init(&ux);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS); /* HOME route 0 */
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    ux.selection=0u;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_PASSTHROUGH_APPLIED);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    ux.selection=1u;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(invoke(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_APPLY_CUSTOM);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection=2u;
    assert(invoke(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(invoke(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
}
static void test_help_navigation_lock(void) {
    blu2usb_ux_model_t ux;init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;ux.selection=2u;
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert(ux.screen==BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS && ux.selection==2u);
    tap(&ux,BLU2USB_CONTROL_KEY_Y);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_HOME);
}
static void test_custom_edit_still_working(void) {
    blu2usb_ux_model_t ux;init(&ux);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;ux.selection=1u;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_LEFT_WILL_BECOME);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    const blu2usb_ux_command_t command=invoke(&ux,BLU2USB_CONTROL_KEY_A);
    assert(command.kind==BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(command.source==BLU2USB_MOUSE_SOURCE_LEFT);
    assert(command.target==BLU2USB_MOUSE_TARGET_RIGHT);
    assert(ux.screen==BLU2USB_SCREEN_EDIT_CUSTOM);
}
int main(void) {
    test_replacement_menu_and_help();
    test_connected_route_and_persistence_command_boundary();
    test_help_navigation_lock();
    test_custom_edit_still_working();
    return 0;
}
