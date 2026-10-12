#include <assert.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"
static void render(blu2usb_ux_model_t *u, blu2usb_ui_frame_t *f) {
    blu2usb_ui_project(u,f); blu2usb_ui_enforce_applied_visual_contract(u,f);
}
static void check(blu2usb_ui_frame_t *f,unsigned r,unsigned c,blu2usb_ui_tone_t t) {
    assert(f->cells[r][c].character!=' ' && f->cells[r][c].tone==t);
}
int main(void) {
    blu2usb_ux_model_t u; blu2usb_ui_frame_t f;
    blu2usb_ux_set_mouse_connected(true); blu2usb_ux_init(&u);
    u.active_profile=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    u.custom_dirty=false; u.screen=BLU2USB_SCREEN_EDIT_CUSTOM;
    u.selection=0u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    u.selection=1u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    u.screen=BLU2USB_SCREEN_LEFT_WILL_BECOME;
    u.custom_source=BLU2USB_MOUSE_SOURCE_LEFT;
    u.custom_targets[0]=BLU2USB_MOUSE_TARGET_LEFT;
    u.selection=0u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    u.selection=1u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    u.screen=BLU2USB_SCREEN_SHORTCUTS_1;
    u.editing_combination=0u; u.shortcut_assignments[0]=1u;
    u.selection=0u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    u.selection=1u; render(&u,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    blu2usb_ux_set_saved_device_count(&u,1u);
    blu2usb_ux_set_saved_connected_bond(&u,0);
    blu2usb_ux_set_saved_mouse_name(&u,0,"Mouse Active");
    blu2usb_ux_set_current_mouse_name(&u,"Mouse Active");
    u.screen=BLU2USB_SCREEN_SAVED_DEVICES;
    u.selection=0u; render(&u,&f);
    check(&f,1u,0u,BLU2USB_UI_TONE_CURRENT);
    check(&f,5u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    check(&f,7u,0u,BLU2USB_UI_TONE_ACTIONABLE);
    return 0;
}
