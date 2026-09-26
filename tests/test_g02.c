#include <assert.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/profiles/custom_template.h"

static void press_release(blu2usb_ux_model_t *ux, blu2usb_control_t c) {
    (void)blu2usb_ux_input(ux, c, true);
    (void)blu2usb_ux_input(ux, c, false);
}

static blu2usb_ux_command_t press_release_cmd(blu2usb_ux_model_t *ux, blu2usb_control_t c) {
    (void)blu2usb_ux_input(ux, c, true);
    return blu2usb_ux_input(ux, c, false);
}

static void test_action_on_release_and_wrap(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(ux.selection == 0);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, true);
    assert(ux.selection == 0);
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_JOY_DOWN, false);
    assert(ux.selection == 1);
    ux.selection = 0;
    press_release(&ux, BLU2USB_CONTROL_JOY_UP);
    assert(ux.selection == 2);
    press_release(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 0);
}

static void test_custom_editor_offline_and_escape_target(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    /* HOME now routes selection 0 to the legacy Mouse Options destination
     * until HOPE-10 replaces it in-place. Enter directly to keep this G02
     * custom-editor regression independent from the evolving HOME shell. */
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.selection = 3;
    press_release(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(blu2usb_ux_option_count(&ux) == 5);
    blu2usb_ux_set_custom_target(&ux, BLU2USB_MOUSE_SOURCE_FORWARD, BLU2USB_MOUSE_TARGET_ESCAPE);
    ux.selection = 3;
    press_release(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_FORWARD_WILL_BECOME);
    assert(ux.selection == BLU2USB_MOUSE_TARGET_ESCAPE);
    assert(blu2usb_ux_option_count(&ux) == 6);
    const blu2usb_screen_template_t *t = blu2usb_ux_screen_template(ux.screen);
    assert(strcmp(t->rows[1], " LEFT") == 0);
    assert(strcmp(t->rows[2], " RIGHT") == 0);
    assert(strcmp(t->rows[3], " MIDDLE") == 0);
    assert(strcmp(t->rows[4], " ESCAPE") == 0);
    assert(strcmp(t->rows[5], " FORWARD") == 0);
    assert(strcmp(t->rows[6], " BACKWARD") == 0);
    blu2usb_ux_command_t cmd = press_release_cmd(&ux, BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind == BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(cmd.source == BLU2USB_MOUSE_SOURCE_FORWARD);
    assert(cmd.target == BLU2USB_MOUSE_TARGET_ESCAPE);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
}

static void test_custom_template_transaction(void) {
    blu2usb_custom_template_editor_t e;
    blu2usb_custom_template_editor_init(&e);
    assert(e.committed.target[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_LEFT);
    assert(e.committed.target[BLU2USB_MOUSE_SOURCE_FORWARD] == BLU2USB_MOUSE_TARGET_FORWARD);
    blu2usb_custom_template_begin(&e);
    assert(blu2usb_custom_template_set_draft(&e, BLU2USB_MOUSE_SOURCE_LEFT, BLU2USB_MOUSE_TARGET_ESCAPE));
    assert(e.draft.target[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_ESCAPE);
    assert(e.committed.target[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_LEFT);
    blu2usb_custom_template_commit(&e);
    assert(e.committed.target[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_ESCAPE);
    blu2usb_custom_template_begin(&e);
    assert(blu2usb_custom_template_set_draft(&e, BLU2USB_MOUSE_SOURCE_RIGHT, BLU2USB_MOUSE_TARGET_FORWARD));
    blu2usb_custom_template_cancel(&e);
    assert(e.committed.target[BLU2USB_MOUSE_SOURCE_RIGHT] == BLU2USB_MOUSE_TARGET_RIGHT);
}

static void test_saved_pagination_wrap(void) {
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    blu2usb_ux_set_saved_device_count(&ux, 6);
    assert(ux.saved_pages == 6 && ux.saved_page == 0);
    press_release(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert(ux.saved_page == 5);
    assert(blu2usb_ux_option_count(&ux) == 0);
    press_release(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.saved_page == 0);
    assert(blu2usb_ux_option_count(&ux) == 0);
}

static void test_all_templates_are_9x21(void) {
    for (unsigned s = 0; s < BLU2USB_SCREEN_COUNT; ++s) {
        const blu2usb_screen_template_t *t = blu2usb_ux_screen_template((blu2usb_screen_id_t)s);
        assert(t != 0);
        for (unsigned r = 0; r < 9; ++r) {
            assert(t->rows[r] != 0);
            assert(strlen(t->rows[r]) <= 21);
        }
    }
}

int main(void) {
    test_action_on_release_and_wrap();
    test_custom_editor_offline_and_escape_target();
    test_custom_template_transaction();
    test_saved_pagination_wrap();
    test_all_templates_are_9x21();
    return 0;
}
