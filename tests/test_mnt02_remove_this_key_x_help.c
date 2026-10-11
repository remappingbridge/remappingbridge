/* MNT-02: replace misleading KEY Y HELP with a working KEY X HELP
 * while preserving locked/pending bond-removal safety and v3.0.1 Help policy. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    const blu2usb_ux_command_t down = blu2usb_ux_input(ux,key,true);
    assert(down.kind == BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux,key,false);
}
static void screen_row(const blu2usb_ux_model_t *ux, unsigned row, const char *want)
{
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(ux,&frame);
    blu2usb_ui_enforce_applied_visual_contract(ux,&frame);
    assert(row < BLU2USB_RENDERER_TEXT_ROWS);
    char have[BLU2USB_RENDERER_TEXT_COLS+1u];
    for (unsigned c=0;c<BLU2USB_RENDERER_TEXT_COLS;++c)
        have[c]=frame.cells[row][c].character;
    unsigned n=BLU2USB_RENDERER_TEXT_COLS;
    while(n && have[n-1]==' ') --n;
    have[n]='\0';
    assert(strcmp(have,want)==0);
    if (row==0) {
        assert(frame.cells[0][0].tone==BLU2USB_UI_TONE_TITLE);
        assert(blu2usb_renderer_tone_rgb565(frame.cells[0][0].tone)==
               BLU2USB_COLOR_MAGENTA);
    }
}
static void setup(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux,3u);
    blu2usb_ux_set_saved_mouse_name(ux,0u,"Mouse Alpha");
    blu2usb_ux_set_saved_mouse_name(ux,1u,"Logitech Lift");
    blu2usb_ux_set_saved_mouse_name(ux,2u,"Mouse Gamma");
    blu2usb_ux_set_current_mouse_name(ux,"Logitech Lift");
    blu2usb_ux_set_saved_connected_bond(ux,1);
    assert(ux->screen==BLU2USB_SCREEN_HOME);
    tap(ux,BLU2USB_CONTROL_JOY_DOWN);
    tap(ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux->selection==2u);
    tap(ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen==BLU2USB_SCREEN_SAVED_DEVICES);
    screen_row(ux,0,"1 OF 3");
    tap(ux,BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux->saved_page==1u);
    assert(blu2usb_ux_saved_bond_for_page(ux,1u)==0);
    tap(ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen==BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux->remove_target_bond==0);
    screen_row(ux,0,"REMOVE THIS MOUSE");
    screen_row(ux,8,"KEY X: HELP");
}
static void test_key_x_opens_help_and_back_preserves_bond(void)
{
    blu2usb_ux_model_t ux; setup(&ux);
    const unsigned old_page=ux.saved_page;
    const int old_bond=ux.remove_target_bond;
    assert(ux.return_screen!=BLU2USB_SCREEN_REMOVE_THIS);
    assert(blu2usb_ux_input(&ux,BLU2USB_CONTROL_KEY_X,true).kind==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_REMOVE_THIS);
    assert(blu2usb_ux_input(&ux,BLU2USB_CONTROL_KEY_X,false).kind==
           BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_HELP_REMOVE_THIS);
    assert(ux.return_screen==BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond==old_bond);
    assert(ux.saved_page==old_page);
    screen_row(&ux,0,"REMOVE MOUSE HELP");
    screen_row(&ux,8,"ANY KEY: BACK"); /* MNT-05 owns a future revision */
    assert(tap(&ux,BLU2USB_CONTROL_KEY_B).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond==old_bond && ux.saved_page==old_page);
    assert(!ux.remove_pending && ux.saved_device_count==3);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_B).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_SAVED_DEVICES);
    assert(ux.saved_page==old_page);
    assert(ux.remove_target_bond==-1);
    screen_row(&ux,5," REMOVE DEVICE"); /* MNT-01 stays preserved */
}
static void test_key_y_locks_not_help(void)
{
    blu2usb_ux_model_t ux; setup(&ux);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_Y).kind==BLU2USB_UX_COMMAND_NONE);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_REMOVE_THIS);
    assert(ux.remove_target_bond==0 && !ux.remove_pending);
    /* First input consumes unlock rather than triggering REMOVE. */
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_HOME);
    assert(ux.saved_device_count==3u);
    assert(!ux.remove_pending);
}
static void test_pending_blocks_help_lock_and_cancel(void)
{
    blu2usb_ux_model_t ux; setup(&ux);
    const blu2usb_ux_command_t c=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(c.kind==BLU2USB_UX_COMMAND_REMOVE_MOUSE);
    assert(c.saved_bond==0 && ux.remove_pending);
    assert(ux.screen==BLU2USB_SCREEN_REMOVE_THIS);
    for (unsigned k=0;k<3u;++k) {
        assert(tap(&ux,BLU2USB_CONTROL_KEY_X).kind==BLU2USB_UX_COMMAND_NONE);
        assert(tap(&ux,BLU2USB_CONTROL_KEY_Y).kind==BLU2USB_UX_COMMAND_NONE);
        assert(tap(&ux,BLU2USB_CONTROL_KEY_B).kind==BLU2USB_UX_COMMAND_NONE);
        assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
        assert(ux.screen==BLU2USB_SCREEN_REMOVE_THIS);
        assert(ux.remove_target_bond==0);
        assert(ux.saved_device_count==3u && ux.remove_pending);
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
    }
}
static void test_other_help_remains_unchanged(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert(ux.screen==BLU2USB_SCREEN_HELP_HOME_CONNECTED);
    screen_row(&ux,8,"ANY KEY: BACK");
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_HOME);
}
int main(void)
{
    test_key_x_opens_help_and_back_preserves_bond();
    test_key_y_locks_not_help();
    test_pending_blocks_help_lock_and_cancel();
    test_other_help_remains_unchanged();
    return 0;
}
