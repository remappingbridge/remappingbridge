#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t c)
{
    (void)blu2usb_ux_input(ux, c, true);
    return blu2usb_ux_input(ux, c, false);
}

static void init_connected(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 1u);
    blu2usb_ux_set_saved_connected_bond(ux, 0);
    blu2usb_ux_set_movement(ux, (blu2usb_movement_settings_t){8u,4u,1u});
}

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *f)
{
    blu2usb_ui_project(ux, f);
    blu2usb_ui_enforce_applied_visual_contract(ux, f);
}

static void assert_text(blu2usb_ui_frame_t *f, unsigned row, const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned i = 0u; i < BLU2USB_RENDERER_TEXT_COLS; ++i)
        actual[i] = f->cells[row][i].character;
    unsigned end = BLU2USB_RENDERER_TEXT_COLS;
    while (end > 0u && actual[end - 1u] == ' ') --end;
    actual[end] = '\0';
    assert(strcmp(actual, expected) == 0);
}

static void assert_tone(blu2usb_ui_frame_t *f, unsigned row, blu2usb_ui_tone_t tone)
{
    bool found = false;
    for (unsigned i = 0; i < BLU2USB_RENDERER_TEXT_COLS; ++i) {
        if (f->cells[row][i].character == ' ') continue;
        found = true;
        assert(f->cells[row][i].tone == tone);
    }
    assert(found);
}

static const char *const labels[6][2] = {
    {" MODIFIER RELEASED IS", " MOD + MIDDLE IS"},
    {" MOD + LEFT CLICK IS", " MOD + RIGHT CLICK IS"},
    {" MOD + FORWARD IS", " MOD + BACKWARD IS"},
    {" MOD + WHEEL UP IS", " MOD + WHEEL DOWN IS"},
    {" MOD + MOVE LEFT IS", " MOD + MOVE RIGHT IS"},
    {" MOD + MOVE UP IS", " MOD + MOVE DOWN IS"}
};

static void assert_page(blu2usb_ux_model_t *ux, unsigned page, unsigned focus)
{
    blu2usb_ui_frame_t f;
    project(ux, &f);
    char title[22];
    snprintf(title, sizeof title, "COMBINATIONS %u OF 6", page+1u);
    assert(ux->screen == (blu2usb_screen_id_t)(BLU2USB_SCREEN_COMBINATIONS_1 + page));
    assert(ux->selection == focus);
    assert(blu2usb_ux_option_count(ux) == 2u);
    assert(f.hint_start_row == 6u);
    assert_text(&f, 0u, title);
    assert_text(&f, 1u, labels[page][0]);
    assert_text(&f, 2u, " NOT CUSTOMIZED");
    assert_text(&f, 3u, labels[page][1]);
    assert_text(&f, 4u, " NOT CUSTOMIZED");
    assert_text(&f, 5u, "");
    assert_text(&f, 6u, "JOY LEFT\\RIGHT: PAGE");
    assert_text(&f, 7u, "JOY PRESS: ACCESS");
    assert_text(&f, 8u, "KEY A: RESET SELECTED");

    assert_tone(&f, 0u, BLU2USB_UI_TONE_TITLE);
    assert_tone(&f, 1u, focus == 0u ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE);
    assert_tone(&f, 2u, focus == 0u ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE);
    assert_tone(&f, 3u, focus == 1u ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE);
    assert_tone(&f, 4u, focus == 1u ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE);
    assert(blu2usb_ux_selected_combination(ux) == (blu2usb_modifier_combo_id_t)(page*2u+focus));
}

static void test_route_and_all_six_layouts(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    assert(blu2usb_ux_option_count(&ux) == 3u);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 2u);
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert_page(&ux, 0u, 0u);

    for (unsigned page = 0u; page < 6u; ++page) {
        assert_page(&ux, page, 0u);
        assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
        assert_page(&ux, page, 0u);
        assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
        assert_page(&ux, page, 0u);
        tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
        assert_page(&ux, page, 1u);
        assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
        assert_page(&ux, page, 1u);
        tap(&ux, BLU2USB_CONTROL_JOY_UP);
        assert_page(&ux, page, 0u);
        tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    }
    assert_page(&ux, 0u, 0u); /* 6 → 1 wrap */
    tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    assert_page(&ux, 5u, 0u); /* 1 → 6 wrap */

    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert_page(&ux, 5u, 1u);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert_page(&ux, 0u, 1u); /* selected pair survives paging */
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert_page(&ux, 0u, 0u);
    tap(&ux, BLU2USB_CONTROL_JOY_UP);
    assert_page(&ux, 0u, 1u); /* two-item vertical wrap */

    assert(tap(&ux, BLU2USB_CONTROL_KEY_B).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS && ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert_page(&ux, 0u, 0u); /* reentry starts at first slot */
    assert(ux.movement_current.horizontal_quarters == 8u);
    assert(ux.movement_current.vertical_quarters == 4u);
    assert(ux.movement_current.rotation_quadrants == 1u);
}

static void test_lock_and_no_mutations(void)
{
    blu2usb_ux_model_t ux;
    init_connected(&ux);
    ux.screen = BLU2USB_SCREEN_COMBINATIONS_3;
    ux.selection = 1u;
    const blu2usb_mouse_profile_kind_t before = ux.active_profile;
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
    assert_page(&ux, 2u, 1u);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_X).kind == BLU2USB_UX_COMMAND_NONE);
    assert_page(&ux, 2u, 1u); /* No unapproved Help screen */
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert_page(&ux, 2u, 1u);
    assert(ux.active_profile == before);
    assert(!ux.custom_dirty);

    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, true);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    (void)blu2usb_ux_input(&ux, BLU2USB_CONTROL_KEY_Y, false);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen == BLU2USB_SCREEN_HOME); /* unlock consumed */
    assert(ux.active_profile == before);
}

static void test_inventory_and_stable_ids(void)
{
    assert(BLU2USB_SCREEN_COUNT == 40);
    assert(BLU2USB_MOD_COMBO_COUNT == 12);
    assert(BLU2USB_MOD_COMBO_RELEASED == 0);
    assert(BLU2USB_MOD_COMBO_MIDDLE == 1);
    assert(BLU2USB_MOD_COMBO_LEFT == 2);
    assert(BLU2USB_MOD_COMBO_RIGHT == 3);
    assert(BLU2USB_MOD_COMBO_FORWARD == 4);
    assert(BLU2USB_MOD_COMBO_BACKWARD == 5);
    assert(BLU2USB_MOD_COMBO_WHEEL_UP == 6);
    assert(BLU2USB_MOD_COMBO_WHEEL_DOWN == 7);
    assert(BLU2USB_MOD_COMBO_MOVE_LEFT == 8);
    assert(BLU2USB_MOD_COMBO_MOVE_RIGHT == 9);
    assert(BLU2USB_MOD_COMBO_MOVE_UP == 10);
    assert(BLU2USB_MOD_COMBO_MOVE_DOWN == 11);
    assert(blu2usb_ux_selected_combination(NULL) == BLU2USB_MOD_COMBO_COUNT);
    for (unsigned i = 0u; i < BLU2USB_SCREEN_COUNT; ++i) {
        const blu2usb_screen_template_t *t = blu2usb_ux_screen_template((blu2usb_screen_id_t)i);
        assert(t != NULL);
        for (unsigned j=0; j<9; ++j) {
            assert(t->rows[j] != NULL);
            assert(strlen(t->rows[j]) <= 21u);
        }
    }
}

int main(void)
{
    test_inventory_and_stable_ids();
    test_route_and_all_six_layouts();
    test_lock_and_no_mutations();
    return 0;
}
