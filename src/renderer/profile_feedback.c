#include "blu2usb/renderer/renderer.h"

#include <stdio.h>

static void clear_row(blu2usb_ui_frame_t *frame, uint8_t row)
{
    if (frame == NULL || row >= BLU2USB_RENDERER_TEXT_ROWS) return;
    for (uint8_t column = 0u; column < BLU2USB_RENDERER_TEXT_COLS; ++column) {
        frame->cells[row][column].character = ' ';
        frame->cells[row][column].tone = BLU2USB_UI_TONE_ACTIONABLE;
    }
}

static void set_row_tone(blu2usb_ui_frame_t *frame,
                         uint8_t row,
                         blu2usb_ui_tone_t tone)
{
    if (frame == NULL || row >= BLU2USB_RENDERER_TEXT_ROWS) return;
    for (uint8_t column = 0u; column < BLU2USB_RENDERER_TEXT_COLS; ++column) {
        if (frame->cells[row][column].character != ' ')
            frame->cells[row][column].tone = tone;
    }
}

static void set_row_current_preserving_selection(blu2usb_ui_frame_t *frame,
                                                  uint8_t row)
{
    if (frame == NULL || row >= BLU2USB_RENDERER_TEXT_ROWS) return;
    for (uint8_t column = 0u; column < BLU2USB_RENDERER_TEXT_COLS; ++column) {
        if (frame->cells[row][column].character == ' ') continue;
        if (frame->cells[row][column].tone == BLU2USB_UI_TONE_EMPHASIZED) continue;
        frame->cells[row][column].tone = BLU2USB_UI_TONE_CURRENT;
    }
}

static bool is_success_feedback(blu2usb_screen_id_t screen)
{
    return screen == BLU2USB_SCREEN_PASSTHROUGH_APPLIED;
}

static uint8_t active_profile_row(const blu2usb_ux_model_t *ux)
{
    if (ux == NULL || ux->screen != BLU2USB_SCREEN_MOUSE_OPTIONS) return 0u;
    switch (ux->active_profile) {
    case BLU2USB_MOUSE_PROFILE_PASSTHROUGH: return 1u;
    case BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP: return 2u;
    default: return 0u;
    }
}

static const char *custom_source_name(unsigned source)
{
    static const char *const names[BLU2USB_MOUSE_SOURCE_COUNT] = {
        "LEFT", "RIGHT", "MIDDLE", "FORWARD", "BACKWARD"
    };
    return source < BLU2USB_MOUSE_SOURCE_COUNT ? names[source] : "LEFT";
}

static uint8_t custom_target_row(blu2usb_mouse_target_t target)
{
    /* WILL BECOME display order is NOT the persisted target enum order. */
    switch (target) {
    case BLU2USB_MOUSE_TARGET_LEFT: return 1u;
    case BLU2USB_MOUSE_TARGET_RIGHT: return 2u;
    case BLU2USB_MOUSE_TARGET_MIDDLE: return 3u;
    case BLU2USB_MOUSE_TARGET_FORWARD: return 4u;
    case BLU2USB_MOUSE_TARGET_BACKWARD: return 5u;
    case BLU2USB_MOUSE_TARGET_MODIFIER: return 6u;
    default: return 0u; /* Retired ESCAPE has no WILL BECOME choice. */
    }
}

static const char *custom_target_name(blu2usb_mouse_target_t target)
{
    switch (target) {
    case BLU2USB_MOUSE_TARGET_LEFT: return "LEFT";
    case BLU2USB_MOUSE_TARGET_RIGHT: return "RIGHT";
    case BLU2USB_MOUSE_TARGET_MIDDLE: return "MIDDLE";
    case BLU2USB_MOUSE_TARGET_FORWARD: return "FORWARD";
    case BLU2USB_MOUSE_TARGET_BACKWARD: return "BACKWARD";
    case BLU2USB_MOUSE_TARGET_MODIFIER: return "MODIFIER";
    case BLU2USB_MOUSE_TARGET_ESCAPE: return "ESCAPE"; /* legacy decoder */
    default: return "?";
    }
}

static void project_custom_rows(const blu2usb_ux_model_t *ux,
                                blu2usb_ui_frame_t *frame,
                                bool applied_feedback)
{
    const bool clean_applied =
        ux->active_profile == BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP && !ux->custom_dirty;

    for (uint8_t source = 0u; source < BLU2USB_MOUSE_SOURCE_COUNT; ++source) {
        const uint8_t row = (uint8_t)(source + 1u);
        char text[BLU2USB_RENDERER_TEXT_COLS + 1u];
        (void)snprintf(text, sizeof(text), applied_feedback ? "%s IS %s" : " %s IS %s",
                       custom_source_name(source),
                       custom_target_name(ux->custom_targets[source]));

        clear_row(frame, row);

        const blu2usb_ui_tone_t normal =
            clean_applied || applied_feedback ? BLU2USB_UI_TONE_CURRENT
                                              : BLU2USB_UI_TONE_ACTIONABLE;
        const blu2usb_ui_tone_t tone =
            applied_feedback ? BLU2USB_UI_TONE_CURRENT
            : ux->selection == source ? blu2usb_ui_tone_on_focus(normal)
            : normal;
        (void)blu2usb_ui_frame_set_text(frame, row, 0u, text, tone);
    }

}

void blu2usb_ui_enforce_applied_visual_contract(const blu2usb_ux_model_t *ux,
                                                 blu2usb_ui_frame_t *frame)
{
    if (ux == NULL || frame == NULL) return;

    if (is_success_feedback(ux->screen)) {
        const uint8_t end = frame->hint_start_row < BLU2USB_RENDERER_TEXT_ROWS
            ? frame->hint_start_row
            : BLU2USB_RENDERER_TEXT_ROWS;
        for (uint8_t row = 1u; row < end; ++row)
            set_row_current_preserving_selection(frame, row);
    }

    if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS) {
        /* Rebuild all three option tones from authoritative state on every
         * projection. This prevents a previously-active profile from
         * retaining cyan after another profile is confirmed. */
        for (uint8_t row = 1u; row <= 3u; ++row)
            set_row_tone(frame, row, BLU2USB_UI_TONE_ACTIONABLE);

        const uint8_t profile_row = active_profile_row(ux);
        if (profile_row != 0u)
            set_row_tone(frame, profile_row, BLU2USB_UI_TONE_CURRENT);

        /* MNT-12: cyan means an APPLIED Custom profile has a modifier.
         * An inactive Custom setting or uncommitted draft is not enough. */
        const bool modifier_active = blu2usb_ux_modifier_shortcuts_active(ux);
        if (modifier_active)
            set_row_tone(frame, 3u, BLU2USB_UI_TONE_CURRENT);

        if (ux->selection < 3u) {
            const uint8_t row = (uint8_t)(1u + ux->selection);
            set_row_tone(frame, row,
                         blu2usb_ui_tone_on_focus(
                             (row == profile_row || (row == 3u && modifier_active))
                                 ? BLU2USB_UI_TONE_CURRENT
                                 : BLU2USB_UI_TONE_ACTIONABLE));
        }
    }

    if (ux->screen == BLU2USB_SCREEN_EDIT_CUSTOM)
        project_custom_rows(ux, frame, false);

    if (ux->screen >= BLU2USB_SCREEN_LEFT_WILL_BECOME &&
        ux->screen <= BLU2USB_SCREEN_BACKWARD_WILL_BECOME) {
        /* Rebuild every candidate tone on every frame, then overlay exactly
         * one persisted/draft value and one selection. This avoids stale
         * cyan at the previously selected item after another commit. */
        for (uint8_t row = 1u; row <= 6u; ++row)
            set_row_tone(frame, row, BLU2USB_UI_TONE_ACTIONABLE);
        const uint8_t current_row =
            custom_target_row(ux->custom_targets[ux->custom_source]);
        if (current_row != 0u)
            set_row_tone(frame, current_row, BLU2USB_UI_TONE_CURRENT);
        if (ux->selection < 6u) {
            const uint8_t row = (uint8_t)(1u + ux->selection);
            set_row_tone(frame, row,
                         blu2usb_ui_tone_on_focus(
                             row == current_row ? BLU2USB_UI_TONE_CURRENT
                                                : BLU2USB_UI_TONE_ACTIONABLE));
        }
    }
}
