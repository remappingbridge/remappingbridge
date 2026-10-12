#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row,
                     char text[BLU2USB_RENDERER_TEXT_COLS+1u])
{
    for (unsigned i=0u;i<BLU2USB_RENDERER_TEXT_COLS;++i)
        text[i]=frame->cells[row][i].character;
    unsigned end=BLU2USB_RENDERER_TEXT_COLS;
    while (end>0u && text[end-1u]==' ') --end;
    text[end]='\0';
}
static void check_names(const char *ble_name, const char *want)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t f;
    char rendered[BLU2USB_RENDERER_TEXT_COLS+1u];
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    blu2usb_ux_set_saved_device_count(&ux,1u);
    blu2usb_ux_set_saved_connected_bond(&ux,0);
    blu2usb_ux_set_saved_mouse_name(&ux,0u,ble_name);
    blu2usb_ux_set_current_mouse_name(&ux,ble_name);

    ux.screen=BLU2USB_SCREEN_HOME;
    blu2usb_ui_project(&ux,&f);
    blu2usb_ui_enforce_applied_visual_contract(&ux,&f);
    row_text(&f,0u,rendered);
    if (strcmp(rendered,want)!=0) fprintf(stderr,"BLE=%s got='%s' expected='%s'\\n",ble_name?ble_name:"(null)",rendered,want);
    assert(strcmp(rendered,want)==0);
    assert(strlen(rendered)<=BLU2USB_RENDERER_TEXT_COLS);
    assert(f.cells[0][0].tone==BLU2USB_UI_TONE_TITLE);
    ux.screen=BLU2USB_SCREEN_SAVED_DEVICES;
    ux.saved_page=0u;
    blu2usb_ui_project(&ux,&f);
    blu2usb_ui_enforce_applied_visual_contract(&ux,&f);
    row_text(&f,1u,rendered);
    assert(strcmp(rendered,want)==0);
    assert(strlen(rendered)<=BLU2USB_RENDERER_TEXT_COLS);
    /* Reconnection/page navigation cannot append a second suffix. */
    blu2usb_ux_set_mouse_connected(false);
    blu2usb_ux_set_saved_connected_bond(&ux,-1);
    blu2usb_ui_project(&ux,&f);
    row_text(&f,1u,rendered);
    assert(strcmp(rendered,want)==0);
}
int main(void)
{
    check_names("BTS. 0MOUSE2","BTS. 0MOUSE2");
    check_names("mOuSe2","MOUSE2");
    check_names("MOUSE-123","MOUSE-123");
    check_names("MOUSEPAD","MOUSEPAD");
    check_names("SuPeRmOuSePaD","SUPERMOUSEPAD");
    check_names("ABC_MOUSE_2","ABCMOUSE2");
    check_names("LOGITECH LIFT","LOGITECH LIFT MOUSE");
    check_names("LIFT","LIFT MOUSE");
    check_names("mouse generic","MOUSE GENERIC");
    check_names("UNKNOWN MOUSE","UNKNOWN MOUSE");
    check_names("ABCDEFGHIJKLMNOmouse","ABCDEFGHIJKLMNO");
    check_names("ABCDEFGHIJKLMNOP","ABCDEFGHIJKLMNO MOUSE");
    check_names("***","UNKNOWN MOUSE");
    check_names("","UNKNOWN MOUSE");
    check_names(NULL,"UNKNOWN MOUSE");
    return 0;
}
