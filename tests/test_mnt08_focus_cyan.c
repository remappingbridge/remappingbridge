#include <assert.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void render(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *f) {
    blu2usb_ui_project(ux,f);
    blu2usb_ui_enforce_applied_visual_contract(ux,f);
}
static void check(blu2usb_ui_frame_t *f,unsigned row,unsigned col,
                  blu2usb_ui_tone_t want) {
    assert(row<9u && col<21u && f->cells[row][col].character!=' ');
    assert(f->cells[row][col].tone==want);
}
int main(void) {
    blu2usb_ux_model_t ux; blu2usb_ui_frame_t f;
    blu2usb_ux_set_mouse_connected(true); blu2usb_ux_init(&ux);
    assert(blu2usb_renderer_tone_rgb565(BLU2USB_UI_TONE_CURRENT)==0x07ffu);
    assert(blu2usb_renderer_tone_rgb565(BLU2USB_UI_TONE_SELECTED_CURRENT)==0xd7ffu);
    assert(blu2usb_ui_tone_on_focus(BLU2USB_UI_TONE_CURRENT)==BLU2USB_UI_TONE_SELECTED_CURRENT);
    assert(blu2usb_ui_tone_on_focus(BLU2USB_UI_TONE_ACTIONABLE)==BLU2USB_UI_TONE_EMPHASIZED);
    ux.screen=BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection=0u; render(&ux,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    check(&f,1u,16u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    check(&f,2u,14u,BLU2USB_UI_TONE_CURRENT);
    ux.selection=1u; render(&ux,&f);
    check(&f,1u,16u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,14u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    ux.screen=BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI;
    ux.selection=0u; render(&ux,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    ux.selection=1u; render(&ux,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    ux.screen=BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.active_profile=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux.selection=0u; render(&ux,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_SELECTED_CURRENT);
    ux.selection=1u; render(&ux,&f);
    check(&f,1u,1u,BLU2USB_UI_TONE_CURRENT);
    check(&f,2u,1u,BLU2USB_UI_TONE_EMPHASIZED);
    return 0;
}
