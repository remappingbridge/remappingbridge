#include <assert.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t k)
{
    const blu2usb_ux_command_t down=blu2usb_ux_input(ux,k,true);
    assert(down.kind==BLU2USB_UX_COMMAND_NONE);
    return blu2usb_ux_input(ux,k,false);
}

static void enter_menu(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux,1u);
    ux->screen=BLU2USB_SCREEN_HOME;
    tap(ux,BLU2USB_CONTROL_JOY_DOWN);
    tap(ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen==BLU2USB_SCREEN_MOVEMENT_OPTIONS);
}

int main(void)
{
    blu2usb_ux_model_t ux;
    enter_menu(&ux);
    assert(ux.movement_current.horizontal_quarters==4);
    assert(ux.movement_current.vertical_quarters==4);
    assert(ux.movement_current.rotation_quadrants==0);

    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.selection==0);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection==1); /* 0.75x */
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert(ux.screen==BLU2USB_SCREEN_HELP_SLOW_DPI);
    tap(&ux,BLU2USB_CONTROL_KEY_Y); /* never lock on Help */
    assert(ux.screen==BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI);
    assert(ux.selection==1);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.horizontal_quarters==3);
    assert(cmd.movement.vertical_quarters==4);
    assert(cmd.movement.rotation_quadrants==0);
    /* Firmware confirms persistent TLV write before entering menu. */
    blu2usb_ux_set_movement(&ux,cmd.movement);
    ux.screen=BLU2USB_SCREEN_MOVEMENT_OPTIONS;

    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection==1);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
    tap(&ux,BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen==BLU2USB_SCREEN_VERTICAL_FAST_DPI);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection==1); /* 1.5x */
    cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.horizontal_quarters==3);
    assert(cmd.movement.vertical_quarters==6);
    blu2usb_ux_set_movement(&ux,cmd.movement);
    ux.screen=BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection=2;

    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen==BLU2USB_SCREEN_ROTATION);
    for(unsigned i=0;i<3;i++)tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection==3);
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert(ux.screen==BLU2USB_SCREEN_HELP_ROTATION);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_ROTATION && ux.selection==3);
    cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.rotation_quadrants==3);
    blu2usb_ux_set_movement(&ux,cmd.movement);
    ux.screen=BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection=2;

    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(&ux,&frame);
    assert(frame.cells[1][16].tone==BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[2][14].tone==BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[4][10].tone==BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[3][1].tone==BLU2USB_UI_TONE_EMPHASIZED);

    cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_APPLY_MOVEMENT);
    assert(cmd.movement.horizontal_quarters==4 &&
           cmd.movement.vertical_quarters==4 &&
           cmd.movement.rotation_quadrants==0);

    /* Cancelling changes never emits a persist command. */
    ux.selection=0;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    tap(&ux,BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen==BLU2USB_SCREEN_HORIZONTAL_FAST_DPI);
    cmd=tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(cmd.kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_MOVEMENT_OPTIONS);
    return 0;
}
