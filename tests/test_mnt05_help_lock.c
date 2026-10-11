#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static void tap(blu2usb_ux_model_t *ux, blu2usb_control_t key) {
    (void)blu2usb_ux_input(ux, key, true);
    assert(blu2usb_ux_input(ux, key, false).kind == BLU2USB_UX_COMMAND_NONE);
}
static void all_help(void) {
    const blu2usb_screen_id_t screens[] = {
        BLU2USB_SCREEN_HELP_HOME_CONNECTED, BLU2USB_SCREEN_HOME_SEARCHING_HELP,
        BLU2USB_SCREEN_HOME_RETRY_HELP, BLU2USB_SCREEN_HELP_PAIR_NEW,
        BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW, BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS,
        BLU2USB_SCREEN_HELP_REMOVE_THIS, BLU2USB_SCREEN_HELP_SLOW_DPI,
        BLU2USB_SCREEN_HELP_FAST_DPI, BLU2USB_SCREEN_HELP_ROTATION
    };
    const blu2usb_control_t inert[] = {
        BLU2USB_CONTROL_KEY_A, BLU2USB_CONTROL_KEY_X, BLU2USB_CONTROL_JOY_UP,
        BLU2USB_CONTROL_JOY_DOWN, BLU2USB_CONTROL_JOY_LEFT,
        BLU2USB_CONTROL_JOY_RIGHT, BLU2USB_CONTROL_JOY_PRESS
    };
    for (unsigned i=0; i<sizeof(screens)/sizeof(screens[0]); ++i) {
        blu2usb_ux_model_t ux;
        blu2usb_ux_set_mouse_connected(true);
        blu2usb_ux_init(&ux);
        ux.screen = screens[i];
        ux.return_screen = BLU2USB_SCREEN_HOME;
        ux.return_selection = 2u;
        ux.remove_target_bond = 3;
        blu2usb_ui_frame_t f;
        blu2usb_ui_project(&ux, &f);
        char hint[22];
        for(unsigned j=0;j<21;++j)hint[j]=f.cells[8][j].character;
        hint[21]=0;
        assert(strncmp(hint, "KEY B: BACK", 11)==0);
        for(unsigned j=0;j<sizeof(inert)/sizeof(inert[0]);++j) {
            tap(&ux,inert[j]);
            assert(ux.screen==screens[i]);
            assert(ux.remove_target_bond==3);
        }
        tap(&ux,BLU2USB_CONTROL_KEY_Y);
        assert(ux.screen==screens[i]);
        assert(blu2usb_interaction_is_locked(&ux.interaction));
        tap(&ux,BLU2USB_CONTROL_KEY_A); /* consumed unlock */
        assert(!blu2usb_interaction_is_locked(&ux.interaction));
        assert(ux.screen==BLU2USB_SCREEN_HOME);
        /* Reenter Help separately to verify KEY B navigation; unlock has
         * already returned HOME via the same global path as other screens. */
        ux.screen = screens[i];
        tap(&ux,BLU2USB_CONTROL_KEY_B);
        assert(ux.screen==(screens[i]==BLU2USB_SCREEN_HOME_SEARCHING_HELP?
                          BLU2USB_SCREEN_HOME_RETRY:
                          screens[i]==BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS?
                          BLU2USB_SCREEN_MOUSE_OPTIONS:BLU2USB_SCREEN_HOME));
        assert(ux.remove_target_bond==3);
    }
}
int main(void){all_help();return 0;}
