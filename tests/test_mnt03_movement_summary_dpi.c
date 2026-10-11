#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static void check(const blu2usb_ui_frame_t *f, unsigned row,const char *expected) {
    char b[BLU2USB_RENDERER_TEXT_COLS+1u];
    for(unsigned c=0;c<BLU2USB_RENDERER_TEXT_COLS;c++)b[c]=f->cells[row][c].character;
    unsigned n=BLU2USB_RENDERER_TEXT_COLS;
    while(n && b[n-1]==' ')--n;
    b[n]=0;
    assert(strcmp(b,expected)==0);
}
static blu2usb_ux_command_t tap(blu2usb_ux_model_t *u,blu2usb_control_t k){
    assert(blu2usb_ux_input(u,k,true).kind==BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(u,k,false);
}
int main(void){
    const uint8_t q[]={1,2,3,4,5,6,8,12};
    const char *n[]={"0.25","0.5","0.75","1","1.25","1.5","2","3"};
    blu2usb_ux_model_t u;blu2usb_ui_frame_t f;
    blu2usb_ux_set_mouse_connected(true);blu2usb_ux_init(&u);
    blu2usb_ux_set_saved_device_count(&u,1);
    tap(&u,BLU2USB_CONTROL_JOY_DOWN);
    tap(&u,BLU2USB_CONTROL_JOY_PRESS);
    assert(u.screen==BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    u.selection=2;
    for(unsigned h=0;h<8;h++)for(unsigned v=0;v<8;v++){
        blu2usb_ux_set_movement(&u,(blu2usb_movement_settings_t){q[h],q[v],3});
        blu2usb_ui_project(&u,&f);
        char s[BLU2USB_RENDERER_TEXT_COLS+1u];
        snprintf(s,sizeof(s)," HORIZONTAL DPI %s",n[h]);check(&f,1,s);
        snprintf(s,sizeof(s)," VERTICAL DPI %s",n[v]);check(&f,2,s);
        check(&f,4," ROTATION 270-DEGREE");
        assert(f.cells[0][0].tone==BLU2USB_UI_TONE_TITLE);
        for(unsigned x=0;x<strlen(n[h]);x++)
            assert(f.cells[1][16+x].tone==BLU2USB_UI_TONE_CURRENT);
        for(unsigned x=0;x<strlen(n[v]);x++)
            assert(f.cells[2][14+x].tone==BLU2USB_UI_TONE_CURRENT);
        assert(f.cells[1][16+strlen(n[h])].character==' ');
        assert(f.cells[2][14+strlen(n[v])].character==' ');
    }
    blu2usb_ux_set_movement(&u,blu2usb_movement_defaults());
    u.selection=0;tap(&u,BLU2USB_CONTROL_JOY_PRESS);
    assert(u.screen==BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    blu2usb_ui_project(&u,&f);
    check(&f,2," SLIGHTLY SLOW 0.75x");
    check(&f,4," VERY SLOW     0.25x");
    tap(&u,BLU2USB_CONTROL_JOY_RIGHT);
    blu2usb_ui_project(&u,&f);
    check(&f,1," SLIGHTLY FAST  1.25x");
    check(&f,4," EXTREMELY FAST 3x");
    tap(&u,BLU2USB_CONTROL_KEY_B);
    assert(u.screen==BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    assert(u.movement_current.horizontal_quarters==4);
    blu2usb_ui_project(&u,&f);
    check(&f,1," HORIZONTAL DPI 1");
    check(&f,2," VERTICAL DPI 1");
    u.screen=BLU2USB_SCREEN_SAVED_DEVICES;
    blu2usb_ux_set_saved_movement(&u,0,(blu2usb_movement_settings_t){3,5,2});
    blu2usb_ui_project(&u,&f);
    blu2usb_ui_enforce_applied_visual_contract(&u,&f);
    check(&f,3,"DPI: H 0.75 - V 1.25");
    assert(f.cells[0][0].tone==BLU2USB_UI_TONE_TITLE);
    assert(f.cells[5][1].tone==BLU2USB_UI_TONE_EMPHASIZED);
    return 0;
}
