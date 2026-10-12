/* MNT-13 — the first LCD column marks only the navigable focus.
 * Run after both visual projection passes, exactly as the device does. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

typedef struct {
    blu2usb_screen_id_t screen;
    unsigned options;
} menu_case_t;

static const menu_case_t menus[] = {
    {BLU2USB_SCREEN_HOME, 4u},
    {BLU2USB_SCREEN_HOME_SEARCHING, 2u},
    {BLU2USB_SCREEN_HOME_RETRY, 2u},
    {BLU2USB_SCREEN_MOUSE_OPTIONS, 3u},
    {BLU2USB_SCREEN_EDIT_CUSTOM, 5u},
    {BLU2USB_SCREEN_LEFT_WILL_BECOME, 6u},
    {BLU2USB_SCREEN_RIGHT_WILL_BECOME, 6u},
    {BLU2USB_SCREEN_MIDDLE_WILL_BECOME, 6u},
    {BLU2USB_SCREEN_FORWARD_WILL_BECOME, 6u},
    {BLU2USB_SCREEN_BACKWARD_WILL_BECOME, 6u},
    {BLU2USB_SCREEN_MOVEMENT_OPTIONS, 3u},
    {BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI, 4u},
    {BLU2USB_SCREEN_HORIZONTAL_FAST_DPI, 4u},
    {BLU2USB_SCREEN_VERTICAL_SLOW_DPI, 4u},
    {BLU2USB_SCREEN_VERTICAL_FAST_DPI, 4u},
    {BLU2USB_SCREEN_ROTATION, 4u},
    {BLU2USB_SCREEN_COMBINATIONS_1, 2u},
    {BLU2USB_SCREEN_COMBINATIONS_2, 2u},
    {BLU2USB_SCREEN_COMBINATIONS_3, 2u},
    {BLU2USB_SCREEN_COMBINATIONS_4, 2u},
    {BLU2USB_SCREEN_COMBINATIONS_5, 2u},
    {BLU2USB_SCREEN_COMBINATIONS_6, 2u},
    {BLU2USB_SCREEN_SHORTCUTS_1, 6u},
    {BLU2USB_SCREEN_SHORTCUTS_2, 6u},
    {BLU2USB_SCREEN_SHORTCUTS_3, 6u},
    {BLU2USB_SCREEN_SHORTCUTS_4, 6u},
    {BLU2USB_SCREEN_SHORTCUTS_5, 6u}
};

static unsigned expected_row(blu2usb_screen_id_t screen, unsigned selection)
{
    if (screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS && selection == 2u)
        return 4u; /* Not the informational COUNTERCLOCKWISE row 3. */
    if (screen >= BLU2USB_SCREEN_COMBINATIONS_1 &&
        screen <= BLU2USB_SCREEN_COMBINATIONS_6)
        return 2u + 2u * selection; /* Second line: value/status, never label. */
    return 1u + selection;
}

static void assert_marker(const blu2usb_ui_frame_t *frame, int row)
{
    unsigned count = 0u;
    for (unsigned r = 0u; r < BLU2USB_RENDERER_TEXT_ROWS; ++r) {
        const char c = frame->cells[r][0].character;
        if (c != '|') continue;
        ++count;
        assert(row == (int)r);
        assert(r > 0u && r < frame->hint_start_row);
        assert(frame->cells[r][1].character != ' ');
        /* Disabled UNAVAILABLE keeps its static text; its pipe uses the
         * selected first-line label tone instead. */
        const blu2usb_ui_tone_t expected_tone =
            frame->cells[r][1].tone == BLU2USB_UI_TONE_STATIC
                ? frame->cells[r - 1u][1].tone : frame->cells[r][1].tone;
        assert(frame->cells[r][0].tone == expected_tone);
        assert(frame->cells[r][0].tone == BLU2USB_UI_TONE_EMPHASIZED ||
               frame->cells[r][0].tone == BLU2USB_UI_TONE_SELECTED_CURRENT);
    }
    assert(count == (row < 0 ? 0u : 1u));
}

static void project_with_marker(const blu2usb_ux_model_t *ux,
                                blu2usb_ui_frame_t *frame,
                                int expected)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
    blu2usb_ui_frame_t before = *frame;

    blu2usb_ui_apply_selection_marker(ux, frame);
    for (unsigned r = 0u; r < BLU2USB_RENDERER_TEXT_ROWS; ++r) {
        for (unsigned col = 1u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
            assert(frame->cells[r][col].character ==
                   before.cells[r][col].character);
            assert(frame->cells[r][col].tone == before.cells[r][col].tone);
        }
        if ((int)r != expected)
            assert(frame->cells[r][0].character == before.cells[r][0].character);
        else
            assert(before.cells[r][0].character == ' '); /* Reserved margin. */
    }
    assert_marker(frame, expected);

    /* Re-entry must neither add a second pipe nor damage the line. */
    blu2usb_ui_frame_t once = *frame;
    blu2usb_ui_apply_selection_marker(ux, frame);
    for (unsigned r = 0u; r < BLU2USB_RENDERER_TEXT_ROWS; ++r)
        for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col) {
            assert(frame->cells[r][col].character ==
                   once.cells[r][col].character);
            assert(frame->cells[r][col].tone == once.cells[r][col].tone);
        }
    assert_marker(frame, expected);
}

static void test_all_list_entries_and_invalid_selection(void)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);

    for (unsigned i = 0u; i < sizeof(menus)/sizeof(menus[0]); ++i) {
        const menu_case_t *menu = &menus[i];
        ux.screen = menu->screen;
        assert(blu2usb_ux_option_count(&ux) == menu->options);
        for (unsigned selection = 0u; selection < menu->options; ++selection) {
            ux.selection = selection;
            project_with_marker(&ux, &frame,
                                (int)expected_row(menu->screen, selection));
        }

        /* Out of bounds must not be modulo-wrapped, even on COMBINATIONS. */
        ux.selection = menu->options;
        project_with_marker(&ux, &frame, -1);
    }
}

static void test_no_menu_and_transition_cleanup(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_HOME;
    ux.selection = 2u;
    project_with_marker(&ux, &frame, 3);

    /* Every screen not in the list inventory must have no pipe. */
    for (unsigned screen = 0u; screen < BLU2USB_SCREEN_COUNT; ++screen) {
        ux.screen = (blu2usb_screen_id_t)screen;
        if (screen == BLU2USB_SCREEN_SAVED_DEVICES ||
            blu2usb_ux_option_count(&ux) != 0u)
            continue;
        ux.selection = 0u;
        project_with_marker(&ux, &frame, -1);
    }

    /* Clear/recalculate when switching menu, page, Help, selection. */
    ux.screen = BLU2USB_SCREEN_COMBINATIONS_6;
    ux.selection = 1u;
    project_with_marker(&ux, &frame, 4);
    ux.screen = BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS;
    project_with_marker(&ux, &frame, -1);
    ux.screen = BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    ux.selection = 2u;
    project_with_marker(&ux, &frame, 4);
    ux.selection = 1u;
    project_with_marker(&ux, &frame, 2);
}

static void test_saved_devices_implicit_action(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_SAVED_DEVICES;
    ux.selection = 0u;
    assert(blu2usb_ux_option_count(&ux) == 0u);

    blu2usb_ux_set_saved_device_count(&ux, 0u);
    project_with_marker(&ux, &frame, -1);

    blu2usb_ux_set_saved_device_count(&ux, 1u);
    assert(blu2usb_ux_saved_bond_for_page(&ux, ux.saved_page) >= 0);
    project_with_marker(&ux, &frame, 5);
    assert(frame.cells[1][0].character != '|'); /* Name isn't a choice. */
    assert(frame.cells[2][0].character != '|'); /* Profile is informational. */
    assert(frame.cells[3][0].character != '|'); /* DPI is informational. */
    assert(frame.cells[4][0].character != '|'); /* Rotation informational. */

    blu2usb_ux_set_saved_device_count(&ux, 2u);
    for (unsigned page = 0u; page < 2u; ++page) {
        ux.saved_page = page;
        project_with_marker(&ux, &frame, 5);
    }
    blu2usb_ux_set_saved_device_count(&ux, 0u);
    ux.saved_page = 0u;
    project_with_marker(&ux, &frame, -1);
}

static void test_focus_color_not_applied_color(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    blu2usb_ux_init(&ux);
    ux.screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
    ux.active_profile = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    ux.modifier_source = BLU2USB_MOUSE_SOURCE_RIGHT;

    ux.selection = 2u;
    project_with_marker(&ux, &frame, 3);
    assert(frame.cells[3][0].tone == BLU2USB_UI_TONE_SELECTED_CURRENT);

    ux.selection = 0u; /* Passthrough receives focus; Custom stays cyan. */
    project_with_marker(&ux, &frame, 1);
    assert(frame.cells[1][0].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_CURRENT);
    assert(frame.cells[3][1].tone == BLU2USB_UI_TONE_CURRENT);

    ux.screen = BLU2USB_SCREEN_COMBINATIONS_2;
    ux.selection = 1u; /* Modifier RIGHT makes selected RIGHT unavailable. */
    assert(blu2usb_ux_combination_unavailable(&ux, BLU2USB_MOD_COMBO_RIGHT));
    project_with_marker(&ux, &frame, 4);
    assert(frame.cells[3][0].character == ' '); /* First-line label. */
    assert(frame.cells[4][0].character == '|'); /* Second-line status. */
    assert(frame.cells[4][1].tone == BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[4][0].tone == BLU2USB_UI_TONE_EMPHASIZED);

    ux.selection = 0u;
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_LEFT] = 4u;
    project_with_marker(&ux, &frame, 2);
    assert(frame.cells[1][0].character == ' '); /* Label unchanged. */
    assert(frame.cells[2][0].character == '|');
    assert(frame.cells[2][0].tone == BLU2USB_UI_TONE_SELECTED_CURRENT);
    assert(frame.cells[2][1].tone == BLU2USB_UI_TONE_SELECTED_CURRENT);
}

int main(void)
{
    test_all_list_entries_and_invalid_selection();
    test_no_menu_and_transition_cleanup();
    test_saved_devices_implicit_action();
    test_focus_color_not_applied_color();
    puts("MNT-13: all navigable screens and focus-marker contracts PASS");
    return 0;
}
