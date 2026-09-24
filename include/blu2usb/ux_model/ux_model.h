#ifndef BLU2USB_UX_MODEL_UX_MODEL_H
#define BLU2USB_UX_MODEL_UX_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "blu2usb/domain/control.h"
#include "blu2usb/domain/profile.h"
#include "blu2usb/interaction/interaction.h"

#define BLU2USB_UX_MOUSE_NAME_CAPACITY 64u
#define BLU2USB_UX_MAX_SAVED_MICE 8u

typedef enum {
    BLU2USB_SCREEN_HOME = 0,
    BLU2USB_SCREEN_HELP_HOME_CONNECTED,
    BLU2USB_SCREEN_HOME_SEARCHING,
    BLU2USB_SCREEN_HOME_SEARCHING_HELP,
    BLU2USB_SCREEN_HOME_RETRY,
    BLU2USB_SCREEN_HOME_RETRY_HELP,
    BLU2USB_SCREEN_MOUSE_STATUS,
    BLU2USB_SCREEN_OTHER_DEVICES_STATUS,
    BLU2USB_SCREEN_MOUSE_HELP,
    BLU2USB_SCREEN_DEVICES_HELP,
    BLU2USB_SCREEN_MOUSE_OPTIONS,
    BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS,
    BLU2USB_SCREEN_PAIR_MOUSE,
    BLU2USB_SCREEN_HELP_PAIR_NEW,
    BLU2USB_SCREEN_RETRY_PAIR_NEW,
    BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW,
    BLU2USB_SCREEN_MOUSE_SAVED,
    BLU2USB_SCREEN_APPLY_PASSTHROUGH,
    BLU2USB_SCREEN_PASSTHROUGH_APPLIED,
    BLU2USB_SCREEN_APPLY_DEFAULT,
    BLU2USB_SCREEN_DEFAULT_APPLIED,
    BLU2USB_SCREEN_APPLY_ESCAPE,
    BLU2USB_SCREEN_ESCAPE_APPLIED,
    BLU2USB_SCREEN_EDIT_CUSTOM,
    BLU2USB_SCREEN_CUSTOM_APPLIED,
    BLU2USB_SCREEN_LEFT_WILL_BECOME,
    BLU2USB_SCREEN_RIGHT_WILL_BECOME,
    BLU2USB_SCREEN_MIDDLE_WILL_BECOME,
    BLU2USB_SCREEN_FORWARD_WILL_BECOME,
    BLU2USB_SCREEN_BACKWARD_WILL_BECOME,
    BLU2USB_SCREEN_SAVED_DEVICES,
    BLU2USB_SCREEN_LEARN_KEYS,
    BLU2USB_SCREEN_COUNT
} blu2usb_screen_id_t;

typedef enum {
    BLU2USB_UX_COMMAND_NONE = 0,
    BLU2USB_UX_COMMAND_PAIR_MOUSE,
    BLU2USB_UX_COMMAND_APPLY_PASSTHROUGH,
    BLU2USB_UX_COMMAND_APPLY_DEFAULT,
    BLU2USB_UX_COMMAND_APPLY_ESCAPE,
    BLU2USB_UX_COMMAND_APPLY_CUSTOM,
    BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET
} blu2usb_ux_command_kind_t;

typedef struct {
    blu2usb_ux_command_kind_t kind;
    blu2usb_mouse_source_t source;
    blu2usb_mouse_target_t target;
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
    unsigned status_page;
    unsigned saved_page;
    unsigned saved_pages;
    unsigned saved_device_count;
    int saved_connected_bond;
    int saved_front_bond;
    char saved_mouse_names[BLU2USB_UX_MAX_SAVED_MICE][BLU2USB_UX_MOUSE_NAME_CAPACITY];
    char current_mouse_name[BLU2USB_UX_MOUSE_NAME_CAPACITY];
    blu2usb_mouse_profile_kind_t active_profile;
    bool custom_dirty;
    blu2usb_mouse_source_t custom_source;
    blu2usb_mouse_target_t custom_targets[BLU2USB_MOUSE_SOURCE_COUNT];
} blu2usb_ux_model_t;

void blu2usb_ux_init(blu2usb_ux_model_t *ux);
blu2usb_ux_command_t blu2usb_ux_input(blu2usb_ux_model_t *ux, blu2usb_control_t control, bool pressed);
void blu2usb_ux_set_saved_device_count(blu2usb_ux_model_t *ux, unsigned count);
void blu2usb_ux_set_saved_connected_bond(blu2usb_ux_model_t *ux, int bond_index);
void blu2usb_ux_set_saved_mouse_name(blu2usb_ux_model_t *ux, unsigned bond_index, const char *name);
int blu2usb_ux_saved_bond_for_page(const blu2usb_ux_model_t *ux, unsigned page);
void blu2usb_ux_set_current_mouse_name(blu2usb_ux_model_t *ux, const char *name);
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
uint16_t blu2usb_ux_learn_white_span_mask(const blu2usb_ux_model_t *ux);

#endif
