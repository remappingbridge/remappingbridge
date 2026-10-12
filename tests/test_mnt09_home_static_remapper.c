#include <assert.h>
#include <string.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void expect_row(const blu2usb_ui_frame_t *f, unsigned row, const char *wanted)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned i=0u;i<BLU2USB_RENDERER_TEXT_COLS;++i)
        actual[i]=f->cells[row][i].character;
    unsigned n=BLU2USB_RENDERER_TEXT_COLS;
    while(n && actual[n-1u]==' ')--n;
    actual[n]=0;
    assert(strcmp(actual,wanted)==0);
}
static void render(const blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *f)
{
    blu2usb_ui_project(ux,f);
    blu2usb_ui_enforce_applied_visual_contract(ux,f);
}
static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    (void)blu2usb_ux_input(ux,key,true);
    (void)blu2usb_ux_input(ux,key,false);
}
static void check_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ui_frame_t f;
    render(ux,&f);
    assert(f.hint_start_row==6u);
    expect_row(&f,1u," REMAPPING OPTIONS");
    expect_row(&f,2u," MOVEMENT OPTIONS");
    expect_row(&f,3u," SAVED DEVICES");
    expect_row(&f,4u," PAIR NEW MOUSE");
    assert(f.cells[0][0].tone==BLU2USB_UI_TONE_TITLE);
    assert(ux->screen==BLU2USB_SCREEN_HOME);
    assert(blu2usb_ux_option_count(ux)==4u);
}
int main(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    ux.screen=BLU2USB_SCREEN_HOME;
    ux.selection=0u;
    ux.active_profile=BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    check_home(&ux);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_HOME);
    ux.active_profile=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    ux.selection=0u;
    check_home(&ux);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_HOME);
    ux.active_profile=BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP; /* legacy restored profile */
    ux.selection=0u;
    check_home(&ux);
    return 0;
}
