#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void check_profile(blu2usb_ux_model_t *ux, unsigned page,
                          const char *expected)
{
    ux->saved_page = page;
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(ux, &frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, &frame);
    char row[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned i=0u; i<BLU2USB_RENDERER_TEXT_COLS; ++i)
        row[i]=frame.cells[2][i].character;
    row[BLU2USB_RENDERER_TEXT_COLS] = '\0';
    unsigned end=BLU2USB_RENDERER_TEXT_COLS;
    while(end>0u && row[end-1u]==' ') row[--end]='\0';
    assert(strcmp(row, expected)==0);
}
int main(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux,2u);
    blu2usb_ux_set_saved_profile(&ux,0u,BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP);
    blu2usb_ux_set_saved_profile(&ux,1u,BLU2USB_MOUSE_PROFILE_PASSTHROUGH);
    ux.screen=BLU2USB_SCREEN_SAVED_DEVICES;
    ux.active_profile=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    check_profile(&ux,0u,"PROFILE: CUSTOM");
    check_profile(&ux,1u,"PROFILE: PASSTHROUGH");
    ux.active_profile=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    check_profile(&ux,0u,"PROFILE: CUSTOM");
    check_profile(&ux,1u,"PROFILE: PASSTHROUGH");
    blu2usb_ux_set_saved_device_count(&ux,1u);
    assert(ux.saved_profile_kinds[1]==BLU2USB_MOUSE_PROFILE_PASSTHROUGH);
    return 0;
}
