#ifndef BLU2USB_UX_MODEL_UX_MODEL_H
#define BLU2USB_UX_MODEL_UX_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "blu2usb/domain/control.h"
#include "blu2usb/domain/profile.h"
#include "blu2usb/interaction/interaction.h"
#include "blu2usb/movement/movement.h"

#define BLU2USB_UX_MOUSE_NAME_CAPACITY 64u
#define BLU2USB_UX_MAX_SAVED_MICE 8u

typedef enum {
    BLU2USB_SCREEN_HOME = 0,
    BLU2USB_SCREEN_HELP_HOME_CONNECTED,
    BLU2USB_SCREEN_HOME_SEARCHING,
    BLU2USB_SCREEN_HOME_SEARCHING_HELP,
    BLU2USB_SCREEN_HOME_RETRY,
    BLU2USB_SCREEN_HOME_RETRY_HELP,
    BLU2USB_SCREEN_MOUSE_OPTIONS,
    BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS,
    BLU2USB_SCREEN_PAIR_MOUSE,
    BLU2USB_SCREEN_HELP_PAIR_NEW,
    BLU2USB_SCREEN_RETRY_PAIR_NEW,
    BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW,
    BLU2USB_SCREEN_MOUSE_SAVED,
    BLU2USB_SCREEN_APPLY_PASSTHROUGH,
    BLU2USB_SCREEN_PASSTHROUGH_APPLIED,
    BLU2USB_SCREEN_EDIT_CUSTOM,
    BLU2USB_SCREEN_LEFT_WILL_BECOME,
    BLU2USB_SCREEN_RIGHT_WILL_BECOME,
    BLU2USB_SCREEN_MIDDLE_WILL_BECOME,
    BLU2USB_SCREEN_FORWARD_WILL_BECOME,
    BLU2USB_SCREEN_BACKWARD_WILL_BECOME,
    BLU2USB_SCREEN_SAVED_DEVICES,
    BLU2USB_SCREEN_REMOVE_THIS,
    BLU2USB_SCREEN_HELP_REMOVE_THIS,
    BLU2USB_SCREEN_SEARCHING_FIRST,
    BLU2USB_SCREEN_MOVEMENT_OPTIONS,
    BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI,
    BLU2USB_SCREEN_HORIZONTAL_FAST_DPI,
    BLU2USB_SCREEN_VERTICAL_SLOW_DPI,
    BLU2USB_SCREEN_VERTICAL_FAST_DPI,
    BLU2USB_SCREEN_ROTATION,
    BLU2USB_SCREEN_HELP_SLOW_DPI,
    BLU2USB_SCREEN_HELP_FAST_DPI,
    BLU2USB_SCREEN_HELP_ROTATION,
    BLU2USB_SCREEN_COMBINATIONS_1,
    BLU2USB_SCREEN_COMBINATIONS_2,
    BLU2USB_SCREEN_COMBINATIONS_3,
    BLU2USB_SCREEN_COMBINATIONS_4,
    BLU2USB_SCREEN_COMBINATIONS_5,
    BLU2USB_SCREEN_COMBINATIONS_6,
    BLU2USB_SCREEN_SHORTCUTS_1,
    BLU2USB_SCREEN_SHORTCUTS_2,
    BLU2USB_SCREEN_SHORTCUTS_3,
    BLU2USB_SCREEN_SHORTCUTS_4,
    BLU2USB_SCREEN_SHORTCUTS_5,
    BLU2USB_SCREEN_COUNT
} blu2usb_screen_id_t;

/* Stable modifier-combination ordinals.  Slots/assignments are added in
 * MOD-03; MOD-02 presents the editor with all slots NOT CUSTOMIZED. */
typedef enum {
    BLU2USB_MOD_COMBO_RELEASED = 0,
    BLU2USB_MOD_COMBO_MIDDLE,
    BLU2USB_MOD_COMBO_LEFT,
    BLU2USB_MOD_COMBO_RIGHT,
    BLU2USB_MOD_COMBO_FORWARD,
    BLU2USB_MOD_COMBO_BACKWARD,
    BLU2USB_MOD_COMBO_WHEEL_UP,
    BLU2USB_MOD_COMBO_WHEEL_DOWN,
    BLU2USB_MOD_COMBO_MOVE_LEFT,
    BLU2USB_MOD_COMBO_MOVE_RIGHT,
    BLU2USB_MOD_COMBO_MOVE_UP,
    BLU2USB_MOD_COMBO_MOVE_DOWN,
    BLU2USB_MOD_COMBO_COUNT
} blu2usb_modifier_combo_id_t;

#define BLU2USB_COMBINATIONS_PAGE_COUNT 6u
#define BLU2USB_COMBINATIONS_PER_PAGE 2u
#define BLU2USB_SHORTCUTS_PAGE_COUNT 5u
#define BLU2USB_SHORTCUTS_PER_PAGE 6u

typedef enum {
    BLU2USB_UX_COMMAND_NONE = 0,
    BLU2USB_UX_COMMAND_PAIR_MOUSE,
    BLU2USB_UX_COMMAND_APPLY_PASSTHROUGH,
    BLU2USB_UX_COMMAND_APPLY_CUSTOM,
    BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET,
    BLU2USB_UX_COMMAND_REMOVE_MOUSE,
    BLU2USB_UX_COMMAND_APPLY_MOVEMENT,
    BLU2USB_UX_COMMAND_SET_SHORTCUT,
    BLU2USB_UX_COMMAND_RESET_SHORTCUT
} blu2usb_ux_command_kind_t;

typedef struct {
    blu2usb_ux_command_kind_t kind;
    blu2usb_mouse_source_t source;
    blu2usb_mouse_target_t target;
    int saved_bond;
    blu2usb_movement_settings_t movement;
    uint8_t combination_id;
    uint8_t shortcut_id; /* 1..30 for SET; 0 for RESET */
} blu2usb_ux_command_t;

typedef struct {
    const char *rows[9];
    uint16_t dynamic_rows;
} blu2usb_screen_template_t;

typedef struct {
    blu2usb_interaction_t interaction;
    blu2usb_screen_id_t screen;
    blu2usb_screen_id_t return_screen;
    unsigned return_selection;
    unsigned selection;
    unsigned saved_page;
    unsigned saved_pages;
    unsigned saved_device_count;
    int saved_connected_bond;
    int saved_front_bond;
    int remove_target_bond;
    bool remove_pending;
    char saved_mouse_names[BLU2USB_UX_MAX_SAVED_MICE][BLU2USB_UX_MOUSE_NAME_CAPACITY];
    char current_mouse_name[BLU2USB_UX_MOUSE_NAME_CAPACITY];
    blu2usb_mouse_profile_kind_t active_profile;
    blu2usb_movement_settings_t movement_current;
    blu2usb_movement_settings_t saved_movements[BLU2USB_UX_MAX_SAVED_MICE];
    blu2usb_mouse_profile_kind_t saved_profile_kinds[BLU2USB_UX_MAX_SAVED_MICE];
    uint8_t movement_dpi_selections[4]; /* last selected index per axis/page */
    bool custom_dirty;
    blu2usb_mouse_source_t custom_source;
    blu2usb_mouse_target_t custom_targets[BLU2USB_MOUSE_SOURCE_COUNT];
    uint8_t shortcut_assignments[BLU2USB_MOD_COMBO_COUNT];
    uint8_t editing_combination; /* stable combo ID; 0..11 */
    int8_t modifier_source; /* -1 none, 0..4 source from persisted Custom */
} blu2usb_ux_model_t;

void blu2usb_ux_init(blu2usb_ux_model_t *ux);
blu2usb_ux_command_t blu2usb_ux_input(blu2usb_ux_model_t *ux, blu2usb_control_t control, bool pressed);
void blu2usb_ux_set_saved_device_count(blu2usb_ux_model_t *ux, unsigned count);
void blu2usb_ux_set_saved_connected_bond(blu2usb_ux_model_t *ux, int bond_index);
void blu2usb_ux_set_saved_mouse_name(blu2usb_ux_model_t *ux, unsigned bond_index, const char *name);
int blu2usb_ux_saved_bond_for_page(const blu2usb_ux_model_t *ux, unsigned page);
int blu2usb_ux_saved_page_for_bond(const blu2usb_ux_model_t *ux, int bond_index);
void blu2usb_ux_set_current_mouse_name(blu2usb_ux_model_t *ux, const char *name);
void blu2usb_ux_set_movement(blu2usb_ux_model_t *ux, blu2usb_movement_settings_t settings);
void blu2usb_ux_set_saved_movement(blu2usb_ux_model_t *ux, unsigned logical_bond,
                                   blu2usb_movement_settings_t settings);
void blu2usb_ux_set_saved_profile(blu2usb_ux_model_t *ux, unsigned logical_bond,
                                  blu2usb_mouse_profile_kind_t kind);
void blu2usb_ux_set_custom_target(blu2usb_ux_model_t *ux, blu2usb_mouse_source_t source, blu2usb_mouse_target_t target);
void blu2usb_ux_profile_applied(blu2usb_ux_model_t *ux, blu2usb_mouse_profile_kind_t active_profile);
void blu2usb_ux_restore_profile_state(
    blu2usb_ux_model_t *ux,
    blu2usb_mouse_profile_kind_t active_profile,
    const blu2usb_mouse_target_t custom_targets[BLU2USB_MOUSE_SOURCE_COUNT]);
void blu2usb_ux_set_mouse_connected(bool connected);
bool blu2usb_ux_mouse_connected(void);
const blu2usb_screen_template_t *blu2usb_ux_screen_template(blu2usb_screen_id_t screen);
unsigned blu2usb_ux_option_count(const blu2usb_ux_model_t *ux);
bool blu2usb_ux_is_combinations_screen(blu2usb_screen_id_t screen);
blu2usb_modifier_combo_id_t blu2usb_ux_selected_combination(const blu2usb_ux_model_t *ux);
bool blu2usb_ux_is_shortcuts_screen(blu2usb_screen_id_t screen);
uint8_t blu2usb_ux_selected_shortcut(const blu2usb_ux_model_t *ux);
const char *blu2usb_ux_shortcut_name(uint8_t shortcut_id);
bool blu2usb_ux_combination_unavailable(const blu2usb_ux_model_t *ux, unsigned combination_id);
/* MNT-12: semantic activity of modifier shortcuts, never a draft/color cache. */
bool blu2usb_ux_modifier_shortcuts_active(const blu2usb_ux_model_t *ux);
void blu2usb_ux_set_modifier_source(blu2usb_ux_model_t *ux, int source);
void blu2usb_ux_custom_target_committed(blu2usb_ux_model_t *ux,
    blu2usb_mouse_source_t source,
    const blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT]);
void blu2usb_ux_shortcut_committed(blu2usb_ux_model_t *ux,
    uint8_t combination_id, uint8_t shortcut_id);
uint16_t blu2usb_ux_learn_white_span_mask(const blu2usb_ux_model_t *ux);

#endif
