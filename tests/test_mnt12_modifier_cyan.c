#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "blu2usb/renderer/renderer.h"
#include "blu2usb/ux_model/ux_model.h"

static void project(blu2usb_ux_model_t *ux, blu2usb_ui_frame_t *frame)
{
    blu2usb_ui_project(ux, frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, frame);
}

static void check_row(const blu2usb_ui_frame_t *frame, unsigned row,
                      blu2usb_ui_tone_t expected)
{
    assert(row < BLU2USB_RENDERER_TEXT_ROWS);
    unsigned chars = 0u;
    for (unsigned column = 0u; column < BLU2USB_RENDERER_TEXT_COLS; ++column) {
        if (frame->cells[row][column].character == ' ') continue;
        ++chars;
        if (frame->cells[row][column].tone != expected) {
            fprintf(stderr, "row %u col %u: actual tone %d, expected %d\n",
                    row, column, (int)frame->cells[row][column].tone, (int)expected);
            assert(frame->cells[row][column].tone == expected);
        }
    }
    assert(chars > 0u);
}

static void check_text(const blu2usb_ui_frame_t *frame, unsigned row,
                       const char *expected)
{
    char text[BLU2USB_RENDERER_TEXT_COLS + 1u];
    for (unsigned col = 0u; col < BLU2USB_RENDERER_TEXT_COLS; ++col)
        text[col] = frame->cells[row][col].character;
    unsigned length = BLU2USB_RENDERER_TEXT_COLS;
    while (length > 0u && text[length - 1u] == ' ') --length;
    text[length] = '\0';
    assert(strcmp(text, expected) == 0);
}

static void init(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    ux->modifier_source = -1;
    ux->screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
}

static void test_menu_truth_table_and_selection(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init(&ux);

    assert(blu2usb_renderer_tone_rgb565(BLU2USB_UI_TONE_CURRENT) == 0x07ffu);
    assert(blu2usb_renderer_tone_rgb565(BLU2USB_UI_TONE_SELECTED_CURRENT) == 0xd7ffu);

    for (unsigned profile = 0u; profile < 2u; ++profile) {
        for (unsigned has_modifier = 0u; has_modifier < 2u; ++has_modifier) {
            ux.active_profile = profile ? BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP
                                        : BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
            ux.modifier_source = has_modifier ? (int8_t)BLU2USB_MOUSE_SOURCE_RIGHT : -1;
            const bool active = (profile != 0u && has_modifier != 0u);
            assert(blu2usb_ux_modifier_shortcuts_active(&ux) == active);
            for (unsigned focus = 0u; focus < 3u; ++focus) {
                ux.selection = focus;
                project(&ux, &frame);
                check_text(&frame, 3u, " MODIFIER SHORTCUTS");
                check_row(&frame, 3u, active
                    ? (focus == 2u ? BLU2USB_UI_TONE_SELECTED_CURRENT : BLU2USB_UI_TONE_CURRENT)
                    : (focus == 2u ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE));
                const unsigned profile_row = profile ? 2u : 1u;
                const unsigned other_row = profile ? 1u : 2u;
                check_row(&frame, profile_row, (focus + 1u == profile_row)
                    ? BLU2USB_UI_TONE_SELECTED_CURRENT : BLU2USB_UI_TONE_CURRENT);
                check_row(&frame, other_row, (focus + 1u == other_row)
                    ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE);
                check_row(&frame, 0u, BLU2USB_UI_TONE_TITLE);
                check_row(&frame, 5u, BLU2USB_UI_TONE_ACTIONABLE);
            }
        }
    }

    /* Invalid source cannot turn the indicator on, even with Custom active. */
    ux.active_profile = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    ux.modifier_source = (int8_t)BLU2USB_MOUSE_SOURCE_COUNT;
    assert(!blu2usb_ux_modifier_shortcuts_active(&ux));
    ux.selection = 2u;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_EMPHASIZED);

    /* Draft changes and dirty flag cannot override the applied source. */
    ux.modifier_source = BLU2USB_MOUSE_SOURCE_RIGHT;
    ux.custom_dirty = true;
    for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i)
        ux.custom_targets[i] = BLU2USB_MOUSE_TARGET_LEFT;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_SELECTED_CURRENT);
    ux.custom_targets[0] = BLU2USB_MOUSE_TARGET_MODIFIER;
    ux.modifier_source = -1;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_EMPHASIZED);

    /* Swapping active mice/profiles must not leak color from earlier frame. */
    ux.modifier_source = BLU2USB_MOUSE_SOURCE_RIGHT;
    ux.custom_dirty = false;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_SELECTED_CURRENT);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_EMPHASIZED);
    ux.active_profile = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    project(&ux, &frame);
    check_row(&frame, 3u, BLU2USB_UI_TONE_SELECTED_CURRENT);
}

static void test_combinations_truth_and_focus(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ui_frame_t frame;
    init(&ux);
    for (unsigned i = 0u; i < BLU2USB_MOD_COMBO_COUNT; ++i)
        ux.shortcut_assignments[i] = (uint8_t)(1u + i); /* all assigned */
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_MIDDLE] = 0u;
    ux.modifier_source = BLU2USB_MOUSE_SOURCE_LEFT; /* left is UNAVAILABLE */
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_LEFT] = 4u; /* deliberately assigned but disabled */

    for (unsigned profile = 0u; profile < 2u; ++profile) {
        for (unsigned has_modifier = 0u; has_modifier < 2u; ++has_modifier) {
            ux.active_profile = profile ? BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP
                                        : BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
            ux.modifier_source = has_modifier ? BLU2USB_MOUSE_SOURCE_LEFT : -1;
            const bool active = profile && has_modifier;
            for (unsigned page = 0u; page < BLU2USB_COMBINATIONS_PAGE_COUNT; ++page) {
                ux.screen = (blu2usb_screen_id_t)(BLU2USB_SCREEN_COMBINATIONS_1 + page);
                for (unsigned selected = 0u; selected < 2u; ++selected) {
                    ux.selection = selected;
                    project(&ux, &frame);
                    check_row(&frame, 0u, BLU2USB_UI_TONE_TITLE);
                    check_row(&frame, 6u, BLU2USB_UI_TONE_ACTIONABLE);
                    check_row(&frame, 7u, BLU2USB_UI_TONE_ACTIONABLE);
                    check_row(&frame, 8u, BLU2USB_UI_TONE_ACTIONABLE);
                    for (unsigned pair = 0u; pair < 2u; ++pair) {
                        const unsigned combo = page * BLU2USB_COMBINATIONS_PER_PAGE + pair;
                        const bool focused = (selected == pair);
                        const bool unavailable = blu2usb_ux_combination_unavailable(&ux, combo);
                        const bool assigned = ux.shortcut_assignments[combo] != 0u;
                        const blu2usb_ui_tone_t label = focused
                            ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE;
                        blu2usb_ui_tone_t value = focused
                            ? BLU2USB_UI_TONE_EMPHASIZED : BLU2USB_UI_TONE_ACTIONABLE;
                        if (unavailable) value = BLU2USB_UI_TONE_STATIC;
                        else if (assigned && active)
                            value = focused ? BLU2USB_UI_TONE_SELECTED_CURRENT
                                            : BLU2USB_UI_TONE_CURRENT;
                        check_row(&frame, 1u + 2u * pair, label);
                        check_row(&frame, 2u + 2u * pair, value);
                    }
                }
            }
        }
    }

    /* Exact examples: assigned ESCAPE / NOT CUSTOMIZED; unavailable LEFT / PASTE. */
    ux.active_profile = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    ux.modifier_source = BLU2USB_MOUSE_SOURCE_LEFT;
    ux.screen = BLU2USB_SCREEN_COMBINATIONS_1;
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_RELEASED] = 7u; /* ESCAPE */
    ux.selection = 0u;
    project(&ux, &frame);
    check_text(&frame, 2u, " ESCAPE");
    check_row(&frame, 2u, BLU2USB_UI_TONE_SELECTED_CURRENT);
    check_text(&frame, 4u, " NOT CUSTOMIZED");
    check_row(&frame, 4u, BLU2USB_UI_TONE_ACTIONABLE);

    ux.screen = BLU2USB_SCREEN_COMBINATIONS_2;
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_RIGHT] = 4u; /* PASTE */
    ux.selection = 1u;
    project(&ux, &frame);
    check_text(&frame, 2u, " UNAVAILABLE");
    check_row(&frame, 2u, BLU2USB_UI_TONE_STATIC);
    check_text(&frame, 4u, " PASTE");
    check_row(&frame, 4u, BLU2USB_UI_TONE_SELECTED_CURRENT);

    /* Resetting an assigned value must immediately revert to old gray/white. */
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_RIGHT] = 0u;
    project(&ux, &frame);
    check_text(&frame, 4u, " NOT CUSTOMIZED");
    check_row(&frame, 4u, BLU2USB_UI_TONE_EMPHASIZED);
    ux.shortcut_assignments[BLU2USB_MOD_COMBO_RIGHT] = 4u;
    project(&ux, &frame);
    check_row(&frame, 4u, BLU2USB_UI_TONE_SELECTED_CURRENT);

    ux.screen = BLU2USB_SCREEN_SHORTCUTS_1;
    ux.editing_combination = BLU2USB_MOD_COMBO_RIGHT;
    ux.selection = 0u;
    project(&ux, &frame);
    check_row(&frame, 1u, BLU2USB_UI_TONE_EMPHASIZED); /* focused, not saved */
    check_row(&frame, 2u, BLU2USB_UI_TONE_ACTIONABLE); /* unrelated list unchanged */
    check_row(&frame, 4u, BLU2USB_UI_TONE_CURRENT); /* PASTE is assigned */
}

int main(void)
{
    test_menu_truth_table_and_selection();
    test_combinations_truth_and_focus();
    return 0;
}
