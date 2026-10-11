#include "blu2usb/ux_model/ux_model.h"

#include <string.h>

#define DYN(row) ((uint16_t)(1u << (row)))
#define EMPTY ""

static const blu2usb_screen_template_t screens[BLU2USB_SCREEN_COUNT] = {
    [BLU2USB_SCREEN_HOME] = {{"UNKNOWN MOUSE"," NO REMAP PASSTHROUGH"," MOVEMENT OPTIONS"," SAVED DEVICES"," PAIR NEW MOUSE",EMPTY,"JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP TO REMOVE"},DYN(0)|DYN(1)},
    [BLU2USB_SCREEN_HELP_HOME_CONNECTED] = {{"REMOVE CONNECTED HELP","TO DISCONNECT THE","CURRENTLY CONNECTED","MOUSE NAVIGATE TO:","STEP 1. SAVED DEVICES","STEP 2. REMOVE DEVICE","STEP 3. KEY A: REMOVE",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HOME_SEARCHING] = {{"SEARCHING SAVED MOUSE"," SAVED DEVICES"," PAIR NEW MOUSE",EMPTY,EMPTY,"KEY B: CANCEL SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HOME_SEARCHING_HELP] = {{"HOME SEARCHING HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HOME_RETRY] = {{"DEVICE NOT FOUND"," SAVED DEVICES"," PAIR NEW MOUSE",EMPTY,EMPTY,"KEY A: RETRY SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HOME_RETRY_HELP] = {{"HOME RETRY HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOUSE_OPTIONS] = {{"REMAPPING OPTIONS"," PASSTHROUGH DEFAULT"," CUSTOM REMAPPING"," MODIFIER SHORTCUTS",EMPTY,"JOY UP\\DOWN: SELECT","JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS] = {{"REMAPPING OPTIONS HELP","PASSTHROUGH DEFAULT","KEEPS ORIGINAL BUTTONS","CUSTOM REMAPPING","EDITS MOUSE BUTTONS","MODIFIER SHORTCUTS","CONFIGURES SHORTCUTS",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_PAIR_MOUSE] = {{"PAIR NEW MOUSE","TRYING TO CONNECT","A NEW MOUSE THAT","IS NOT LISTED","IN SAVED DEVICES",EMPTY,"KEY B: CANCEL","KEY X: HELP","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_HELP_PAIR_NEW] = {{"PAIR NEW DEVICE HELP","TO CONNECT A SAVED","DEVICE FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_RETRY_PAIR_NEW] = {{"NEW MOUSE NOT FOUND","NO NEW MOUSE OUTSIDE","THE LIST OF SAVED","DEVICES WAS FOUND",EMPTY,"KEY A: RETRY NEW PAIR","KEY B: BACK TRY SAVED","KEY X: HELP","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW] = {{"MOUSE NOT FOUND HELP","TO CONNECT A SAVED","DEVICE FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOUSE_SAVED] = {{"FIRST MOUSE CONNECTED","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y",EMPTY," KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_APPLY_PASSTHROUGH] = {{"APPLY PASSTHROUGH","ORIGINAL MOUSE","BUTTONS POSITION","ARE NOT ACTIVE",EMPTY,EMPTY,"KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_PASSTHROUGH_APPLIED] = {{"PASSTHROUGH ACTIVE","ORIGINAL MOUSE","BUTTONS POSITION","ARE ACTIVE NOW",EMPTY,EMPTY,EMPTY,"KEY B: BACK","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_EDIT_CUSTOM] = {{"EDIT CUSTOM REMAP"," LEFT IS LEFT"," RIGHT IS RIGHT"," MIDDLE IS MIDDLE"," FORWARD IS FORWARD"," BACKWARD IS BACKWARD",EMPTY,"JOY PRESS: ACCESS","KEY A: APPLY CUSTOM"},DYN(1)|DYN(2)|DYN(3)|DYN(4)|DYN(5)},
    [BLU2USB_SCREEN_LEFT_WILL_BECOME] = {{"LEFT WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_RIGHT_WILL_BECOME] = {{"RIGHT WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_MIDDLE_WILL_BECOME] = {{"MIDDLE WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_FORWARD_WILL_BECOME] = {{"FORWARD WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_BACKWARD_WILL_BECOME] = {{"BACKWARD WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," FORWARD"," BACKWARD"," MODIFIER",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SAVED_DEVICES] = {{"0 OF 0","UNKNOWN MOUSE","PROFILE: PASSTHROUGH","DPI: H 1 - V 1","ROTATION: 0 DEGREE"," REMOVE DEVICE",EMPTY,"JOY RIGHT\\LEFT: PAGE","JOY PRESS: ACCESS"},DYN(0)|DYN(1)|DYN(2)|DYN(3)|DYN(4)},
    [BLU2USB_SCREEN_REMOVE_THIS] = {{"REMOVE THIS MOUSE","UNKNOWN MOUSE","SAVED PAIRING AND","PREFERENCES WILL","BE DELETED.",EMPTY,"KEY A: REMOVE","KEY B: CANCEL","KEY Y: HELP"},DYN(1)},
    [BLU2USB_SCREEN_HELP_REMOVE_THIS] = {{"REMOVE MOUSE HELP","COMPLETELY REMOVE THE","AUTOMATIC CONNECTION","WHEN TURNING ON THE","DEVICE AND DELETE ITS","BUTTON REMAPPING AND","CUSTOM MOVEMENTS",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOVEMENT_OPTIONS] = {{"MOVEMENT OPTIONS"," HORIZONTAL DPI 1X"," VERTICAL DPI 1X"," COUNTERCLOCKWISE"," ROTATION 0-DEGREE",EMPTY,"JOY PRESS: ACCESS","KEY A: RESET ALL","KEY B: BACK"},0},
    [BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI] = {{"HORIZONTAL SLOW DPI"," NORMAL        1x"," SLIGHTLY SLOW 0.75x"," SLOW          0.5x"," VERY SLOW     0.25x","","JOY LEFT\\RIGHT: PAGE","KEY A: APPLY AND BACK","KEY X: HELP SLOW DPI"},0},
    [BLU2USB_SCREEN_HORIZONTAL_FAST_DPI] = {{"HORIZONTAL FAST DPI"," SLIGHTLY FAST  1.25x"," FAST           1.5x"," VERY FAST      2x"," EXTREMELY FAST 3x","","JOY LEFT\\RIGHT: PAGE","KEY A: APPLY AND BACK","KEY X: HELP FAST DPI"},0},
    [BLU2USB_SCREEN_VERTICAL_SLOW_DPI] = {{"VERTICAL SLOW DPI"," NORMAL        1x"," SLIGHTLY SLOW 0.75x"," SLOW          0.5x"," VERY SLOW     0.25x","","JOY LEFT\\RIGHT: PAGE","KEY A: APPLY AND BACK","KEY X: HELP SLOW DPI"},0},
    [BLU2USB_SCREEN_VERTICAL_FAST_DPI] = {{"VERTICAL FAST DPI"," SLIGHTLY FAST  1.25x"," FAST           1.5x"," VERY FAST      2x"," EXTREMELY FAST 3x","","JOY LEFT\\RIGHT: PAGE","KEY A: APPLY AND BACK","KEY X: HELP FAST DPI"},0},
    [BLU2USB_SCREEN_ROTATION] = {{"ROTATION"," 0-DEGREE"," 90-DEGREE"," 180-DEGREE"," 270-DEGREE","","KEY A: APPLY AND BACK","KEY B: CANCEL","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HELP_SLOW_DPI] = {{"SLOW DPI HELP","SLOWER DPI POINTER","ARE RECOMMENDED","FOR MOVEMENTS THAT","REQUIRE GREATER","SENSITIVITY AND","PRECISION","","ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HELP_FAST_DPI] = {{"FAST DPI HELP","FASTER DPI POINTER","IS RECOMMENDED FOR","HIGH-RESOLUTION","DESKTOP AND","WIDESCREEN","ENVIRONMENTS","","ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HELP_ROTATION] = {{"ROTATION HELP","MOUSE MOVEMENT","CHANGES DIRECTION","EVERY 90 DEGREES","FROM THE PREVIOUS","DIRECTION","ANTICLOCKWISE","","ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_COMBINATIONS_1] = {{"COMBINATIONS 1 OF 6"," MODIFIER RELEASED IS"," NOT CUSTOMIZED"," MOD + MIDDLE IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_COMBINATIONS_2] = {{"COMBINATIONS 2 OF 6"," MOD + LEFT CLICK IS"," NOT CUSTOMIZED"," MOD + RIGHT CLICK IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_COMBINATIONS_3] = {{"COMBINATIONS 3 OF 6"," MOD + FORWARD IS"," NOT CUSTOMIZED"," MOD + BACKWARD IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_COMBINATIONS_4] = {{"COMBINATIONS 4 OF 6"," MOD + WHEEL UP IS"," NOT CUSTOMIZED"," MOD + WHEEL DOWN IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_COMBINATIONS_5] = {{"COMBINATIONS 5 OF 6"," MOD + MOVE LEFT IS"," NOT CUSTOMIZED"," MOD + MOVE RIGHT IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_COMBINATIONS_6] = {{"COMBINATIONS 6 OF 6"," MOD + MOVE UP IS"," NOT CUSTOMIZED"," MOD + MOVE DOWN IS"," NOT CUSTOMIZED",EMPTY,"JOY LEFT\\RIGHT: PAGE","JOY PRESS: ACCESS","KEY A: RESET SELECTED"},DYN(2)|DYN(4)},
    [BLU2USB_SCREEN_SHORTCUTS_1] = {{"SHORTCUTS 1-6 OF 30"," COPY"," CUT"," SELECT ALL"," PASTE"," UNDO"," REDO",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SHORTCUTS_2] = {{"SHORTCUTS 7-12 OF 30"," ESCAPE"," SPACE"," ENTER\\RETURN"," BACKSPACE"," DELETE"," RENAME",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SHORTCUTS_3] = {{"SHORTCUTS 13-18 OF 30"," FULL SCREEN"," SCREENSHOT FULL"," SCREENSHOT AREA"," REFRESH"," FIND"," SAVE",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SHORTCUTS_4] = {{"SHORTCUTS 19-24 OF 30"," PLAY\\PAUSE"," NEXT TRACK"," PREVIOUS TRACK"," VOLUME UP"," VOLUME DOWN"," MUTE",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SHORTCUTS_5] = {{"SHORTCUTS 25-30 OF 30"," HOME SCREEN\\PAGE"," ZOOM IN"," ZOOM OUT"," RESET ZOOM"," BRIGHTNESS UP"," BRIGHTNESS DOWN",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SEARCHING_FIRST] = {{"SEARCHING FIRST MOUSE","PRESS TO LEARN KEYS","WHILE WAIT CONNECTION","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y"},0},
};

static blu2usb_ux_command_t no_command(void) {
    blu2usb_ux_command_t cmd = {
        .kind = BLU2USB_UX_COMMAND_NONE,
        .source = BLU2USB_MOUSE_SOURCE_LEFT,
        .target = BLU2USB_MOUSE_TARGET_LEFT,
        .saved_bond = -1,
    };
    return cmd;
}

static bool is_help(blu2usb_screen_id_t screen) {
    return screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED ||
           screen == BLU2USB_SCREEN_HOME_SEARCHING_HELP ||
           screen == BLU2USB_SCREEN_HOME_RETRY_HELP ||
           screen == BLU2USB_SCREEN_HELP_PAIR_NEW ||
           screen == BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW ||
           screen == BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS ||
           screen == BLU2USB_SCREEN_HELP_REMOVE_THIS ||
           screen == BLU2USB_SCREEN_HELP_SLOW_DPI ||
           screen == BLU2USB_SCREEN_HELP_FAST_DPI ||
           screen == BLU2USB_SCREEN_HELP_ROTATION;
}

bool blu2usb_ux_is_combinations_screen(blu2usb_screen_id_t screen)
{
    return screen >= BLU2USB_SCREEN_COMBINATIONS_1 &&
           screen <= BLU2USB_SCREEN_COMBINATIONS_6;
}

blu2usb_modifier_combo_id_t blu2usb_ux_selected_combination(const blu2usb_ux_model_t *ux)
{
    if (ux == NULL || !blu2usb_ux_is_combinations_screen(ux->screen))
        return BLU2USB_MOD_COMBO_COUNT;
    const unsigned page = (unsigned)(ux->screen - BLU2USB_SCREEN_COMBINATIONS_1);
    const unsigned row = ux->selection % BLU2USB_COMBINATIONS_PER_PAGE;
    return (blu2usb_modifier_combo_id_t)(page * BLU2USB_COMBINATIONS_PER_PAGE + row);
}

void blu2usb_ux_set_modifier_source(blu2usb_ux_model_t *ux, int source)
{
    if (ux == NULL) return;
    ux->modifier_source = source >= 0 &&
        source < (int)BLU2USB_MOUSE_SOURCE_COUNT ? (int8_t)source : -1;
}

bool blu2usb_ux_combination_unavailable(const blu2usb_ux_model_t *ux,
                                         unsigned combination_id)
{
    if (ux == NULL || combination_id >= BLU2USB_MOD_COMBO_COUNT) return false;
    switch (combination_id) {
    case BLU2USB_MOD_COMBO_MIDDLE: return ux->modifier_source == BLU2USB_MOUSE_SOURCE_MIDDLE;
    case BLU2USB_MOD_COMBO_LEFT: return ux->modifier_source == BLU2USB_MOUSE_SOURCE_LEFT;
    case BLU2USB_MOD_COMBO_RIGHT: return ux->modifier_source == BLU2USB_MOUSE_SOURCE_RIGHT;
    case BLU2USB_MOD_COMBO_FORWARD: return ux->modifier_source == BLU2USB_MOUSE_SOURCE_FORWARD;
    case BLU2USB_MOD_COMBO_BACKWARD: return ux->modifier_source == BLU2USB_MOUSE_SOURCE_BACKWARD;
    default: return false; /* RELEASED, wheel and move remain available */
    }
}

bool blu2usb_ux_is_shortcuts_screen(blu2usb_screen_id_t screen)
{
    return screen >= BLU2USB_SCREEN_SHORTCUTS_1 &&
           screen <= BLU2USB_SCREEN_SHORTCUTS_5;
}

uint8_t blu2usb_ux_selected_shortcut(const blu2usb_ux_model_t *ux)
{
    if (ux == NULL || !blu2usb_ux_is_shortcuts_screen(ux->screen))
        return 0u;
    return (uint8_t)(
        1u + (unsigned)(ux->screen - BLU2USB_SCREEN_SHORTCUTS_1) *
        BLU2USB_SHORTCUTS_PER_PAGE + ux->selection % BLU2USB_SHORTCUTS_PER_PAGE);
}

const char *blu2usb_ux_shortcut_name(uint8_t shortcut_id)
{
    static const char *const names[31] = {
        "NOT CUSTOMIZED",
        "COPY", "CUT", "SELECT ALL", "PASTE", "UNDO", "REDO",
        "ESCAPE", "SPACE", "ENTER\\RETURN", "BACKSPACE", "DELETE", "RENAME",
        "FULL SCREEN", "SCREENSHOT FULL", "SCREENSHOT AREA", "REFRESH",
        "FIND", "SAVE", "PLAY\\PAUSE", "NEXT TRACK", "PREVIOUS TRACK",
        "VOLUME UP", "VOLUME DOWN", "MUTE", "HOME SCREEN\\PAGE",
        "ZOOM IN", "ZOOM OUT", "RESET ZOOM", "BRIGHTNESS UP", "BRIGHTNESS DOWN"
    };
    return shortcut_id <= 30u ? names[shortcut_id] : "?";
}

void blu2usb_ux_shortcut_committed(blu2usb_ux_model_t *ux,
    uint8_t combination_id, uint8_t shortcut_id)
{
    if (ux == NULL || combination_id >= BLU2USB_MOD_COMBO_COUNT ||
        shortcut_id > 30u) return;
    ux->shortcut_assignments[combination_id] = shortcut_id;
    ux->screen = (blu2usb_screen_id_t)(
        BLU2USB_SCREEN_COMBINATIONS_1 + combination_id / BLU2USB_COMBINATIONS_PER_PAGE);
    ux->selection = combination_id % BLU2USB_COMBINATIONS_PER_PAGE;
}

static bool is_will_become(blu2usb_screen_id_t screen) {
    return screen >= BLU2USB_SCREEN_LEFT_WILL_BECOME && screen <= BLU2USB_SCREEN_BACKWARD_WILL_BECOME;
}

static bool lock_allowed(blu2usb_screen_id_t screen) {
    return !is_help(screen) && screen != BLU2USB_SCREEN_SEARCHING_FIRST;
}

static void enter(blu2usb_ux_model_t *ux, blu2usb_screen_id_t screen) {
    if (screen == BLU2USB_SCREEN_HOME) {
        if (blu2usb_ux_mouse_connected()) screen = BLU2USB_SCREEN_HOME;
        else if (ux->saved_device_count > 0u) screen = BLU2USB_SCREEN_HOME_SEARCHING;
        else screen = BLU2USB_SCREEN_SEARCHING_FIRST;
    }
    ux->screen = screen;
    ux->selection = 0;
}

static unsigned wrap_prev(unsigned value, unsigned count) { return count ? (value + count - 1u) % count : 0u; }
static unsigned wrap_next(unsigned value, unsigned count) { return count ? (value + 1u) % count : 0u; }

static blu2usb_screen_id_t back_target(const blu2usb_ux_model_t *ux) {
    switch (ux->screen) {
    case BLU2USB_SCREEN_HOME: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS:
    case BLU2USB_SCREEN_HELP_REMOVE_THIS: return ux->return_screen;
    case BLU2USB_SCREEN_MOUSE_OPTIONS:
    case BLU2USB_SCREEN_MOVEMENT_OPTIONS: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI:
    case BLU2USB_SCREEN_HORIZONTAL_FAST_DPI:
    case BLU2USB_SCREEN_VERTICAL_SLOW_DPI:
    case BLU2USB_SCREEN_VERTICAL_FAST_DPI:
    case BLU2USB_SCREEN_ROTATION: return BLU2USB_SCREEN_MOVEMENT_OPTIONS;
    case BLU2USB_SCREEN_PAIR_MOUSE:
    case BLU2USB_SCREEN_MOUSE_SAVED: return BLU2USB_SCREEN_MOUSE_OPTIONS;
    case BLU2USB_SCREEN_APPLY_PASSTHROUGH:
    case BLU2USB_SCREEN_EDIT_CUSTOM:
    case BLU2USB_SCREEN_PASSTHROUGH_APPLIED:
    return BLU2USB_SCREEN_MOUSE_OPTIONS;
    case BLU2USB_SCREEN_LEFT_WILL_BECOME:
    case BLU2USB_SCREEN_RIGHT_WILL_BECOME:
    case BLU2USB_SCREEN_MIDDLE_WILL_BECOME:
    case BLU2USB_SCREEN_FORWARD_WILL_BECOME:
    case BLU2USB_SCREEN_BACKWARD_WILL_BECOME: return BLU2USB_SCREEN_EDIT_CUSTOM;
    case BLU2USB_SCREEN_COMBINATIONS_1:
    case BLU2USB_SCREEN_COMBINATIONS_2:
    case BLU2USB_SCREEN_COMBINATIONS_3:
    case BLU2USB_SCREEN_COMBINATIONS_4:
    case BLU2USB_SCREEN_COMBINATIONS_5:
    case BLU2USB_SCREEN_COMBINATIONS_6: return BLU2USB_SCREEN_MOUSE_OPTIONS;
    case BLU2USB_SCREEN_SHORTCUTS_1:
    case BLU2USB_SCREEN_SHORTCUTS_2:
    case BLU2USB_SCREEN_SHORTCUTS_3:
    case BLU2USB_SCREEN_SHORTCUTS_4:
    case BLU2USB_SCREEN_SHORTCUTS_5:
        return (blu2usb_screen_id_t)(BLU2USB_SCREEN_COMBINATIONS_1 + ux->editing_combination / 2u);
    case BLU2USB_SCREEN_SAVED_DEVICES: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_REMOVE_THIS: return BLU2USB_SCREEN_SAVED_DEVICES;
    case BLU2USB_SCREEN_SEARCHING_FIRST: return BLU2USB_SCREEN_SEARCHING_FIRST;
    default: return BLU2USB_SCREEN_HOME;
    }
}

unsigned blu2usb_ux_option_count(const blu2usb_ux_model_t *ux) {
    switch (ux->screen) {
    case BLU2USB_SCREEN_HOME: return 4;
    case BLU2USB_SCREEN_MOVEMENT_OPTIONS: return 3;
    case BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI:
    case BLU2USB_SCREEN_HORIZONTAL_FAST_DPI:
    case BLU2USB_SCREEN_VERTICAL_SLOW_DPI:
    case BLU2USB_SCREEN_VERTICAL_FAST_DPI:
    case BLU2USB_SCREEN_ROTATION: return 4;
    case BLU2USB_SCREEN_HOME_SEARCHING:
    case BLU2USB_SCREEN_HOME_RETRY: return 2;
    case BLU2USB_SCREEN_MOUSE_OPTIONS: return 3;
    case BLU2USB_SCREEN_COMBINATIONS_1:
    case BLU2USB_SCREEN_COMBINATIONS_2:
    case BLU2USB_SCREEN_COMBINATIONS_3:
    case BLU2USB_SCREEN_COMBINATIONS_4:
    case BLU2USB_SCREEN_COMBINATIONS_5:
    case BLU2USB_SCREEN_COMBINATIONS_6: return BLU2USB_COMBINATIONS_PER_PAGE;
    case BLU2USB_SCREEN_SHORTCUTS_1:
    case BLU2USB_SCREEN_SHORTCUTS_2:
    case BLU2USB_SCREEN_SHORTCUTS_3:
    case BLU2USB_SCREEN_SHORTCUTS_4:
    case BLU2USB_SCREEN_SHORTCUTS_5: return BLU2USB_SHORTCUTS_PER_PAGE;
    case BLU2USB_SCREEN_EDIT_CUSTOM: return 5;
    case BLU2USB_SCREEN_LEFT_WILL_BECOME:
    case BLU2USB_SCREEN_RIGHT_WILL_BECOME:
    case BLU2USB_SCREEN_MIDDLE_WILL_BECOME:
    case BLU2USB_SCREEN_FORWARD_WILL_BECOME:
    case BLU2USB_SCREEN_BACKWARD_WILL_BECOME: return 6u;
    case BLU2USB_SCREEN_SAVED_DEVICES: return 0;
    default: return 0;
    }
}

void blu2usb_ux_init(blu2usb_ux_model_t *ux) {
    blu2usb_interaction_init(&ux->interaction);
    ux->screen = BLU2USB_SCREEN_HOME;
    ux->return_screen = BLU2USB_SCREEN_HOME;
    ux->return_selection = 0u;
    ux->selection = 0;
    ux->saved_page = 0;
    ux->saved_pages = 1;
    ux->saved_device_count = 0;
    ux->saved_connected_bond = -1;
    ux->saved_front_bond = -1;
    ux->remove_target_bond = -1;
    ux->remove_pending = false;
    memset(ux->saved_mouse_names, 0, sizeof(ux->saved_mouse_names));
    ux->current_mouse_name[0] = '\0';
    ux->active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux->movement_current = blu2usb_movement_defaults();
    for (unsigned m = 0u; m < BLU2USB_UX_MAX_SAVED_MICE; ++m)
        ux->saved_movements[m] = blu2usb_movement_defaults();
    memset(ux->movement_dpi_selections, 0, sizeof(ux->movement_dpi_selections));
    ux->custom_dirty = false;
    memset(ux->shortcut_assignments, 0, sizeof(ux->shortcut_assignments));
    ux->editing_combination = 0u;
    ux->modifier_source = -1;
    ux->custom_source = BLU2USB_MOUSE_SOURCE_LEFT;
    ux->custom_targets[BLU2USB_MOUSE_SOURCE_LEFT] = BLU2USB_MOUSE_TARGET_LEFT;
    ux->custom_targets[BLU2USB_MOUSE_SOURCE_RIGHT] = BLU2USB_MOUSE_TARGET_RIGHT;
    ux->custom_targets[BLU2USB_MOUSE_SOURCE_MIDDLE] = BLU2USB_MOUSE_TARGET_MIDDLE;
    ux->custom_targets[BLU2USB_MOUSE_SOURCE_FORWARD] = BLU2USB_MOUSE_TARGET_FORWARD;
    ux->custom_targets[BLU2USB_MOUSE_SOURCE_BACKWARD] = BLU2USB_MOUSE_TARGET_BACKWARD;
}

void blu2usb_ux_set_saved_device_count(blu2usb_ux_model_t *ux, unsigned count) {
    if (ux == NULL) return;
    if (count > BLU2USB_UX_MAX_SAVED_MICE) count = BLU2USB_UX_MAX_SAVED_MICE;
    ux->saved_device_count = count;
    ux->saved_pages = count == 0 ? 1u : count;
    if (ux->saved_page >= ux->saved_pages) ux->saved_page = ux->saved_pages - 1u;
    if (ux->saved_connected_bond >= (int)count) ux->saved_connected_bond = -1;
    if (ux->saved_front_bond >= (int)count) ux->saved_front_bond = -1;
    for (unsigned index = count; index < BLU2USB_UX_MAX_SAVED_MICE; ++index)
        ux->saved_mouse_names[index][0] = '\0';
    if (ux->selection >= blu2usb_ux_option_count(ux)) ux->selection = 0;
}

void blu2usb_ux_set_saved_connected_bond(blu2usb_ux_model_t *ux, int bond_index) {
    if (ux == NULL) return;
    if (bond_index >= 0 && (unsigned)bond_index < ux->saved_device_count) {
        ux->saved_connected_bond = bond_index;
        ux->saved_front_bond = bond_index;
        ux->saved_page = 0u;
    } else {
        ux->saved_connected_bond = -1;
    }
}

void blu2usb_ux_set_saved_mouse_name(blu2usb_ux_model_t *ux,
                                     unsigned bond_index,
                                     const char *name) {
    if (ux == NULL || bond_index >= BLU2USB_UX_MAX_SAVED_MICE) return;
    if (name == NULL) {
        ux->saved_mouse_names[bond_index][0] = '\0';
        return;
    }
    size_t i = 0u;
    while (name[i] != '\0' && i + 1u < BLU2USB_UX_MOUSE_NAME_CAPACITY) {
        ux->saved_mouse_names[bond_index][i] = name[i];
        ++i;
    }
    ux->saved_mouse_names[bond_index][i] = '\0';
}

int blu2usb_ux_saved_bond_for_page(const blu2usb_ux_model_t *ux,
                                   unsigned page) {
    if (ux == NULL || page >= ux->saved_device_count) return -1;

    const int front =
        ux->saved_front_bond >= 0 &&
        (unsigned)ux->saved_front_bond < ux->saved_device_count
            ? ux->saved_front_bond
            : -1;
    if (front < 0) return (int)page;
    if (page == 0u) return front;

    unsigned remaining = page - 1u;
    for (unsigned bond = 0u; bond < ux->saved_device_count; ++bond) {
        if ((int)bond == front) continue;
        if (remaining == 0u) return (int)bond;
        --remaining;
    }
    return -1;
}

int blu2usb_ux_saved_page_for_bond(const blu2usb_ux_model_t *ux,
                                    int bond_index) {
    if (ux == NULL || bond_index < 0) return -1;
    for (unsigned page = 0u; page < ux->saved_device_count; ++page)
        if (blu2usb_ux_saved_bond_for_page(ux, page) == bond_index)
            return (int)page;
    return -1;
}

void blu2usb_ux_set_current_mouse_name(blu2usb_ux_model_t *ux, const char *name) {
    if (ux == NULL) return;
    if (name == NULL) {
        ux->current_mouse_name[0] = '\0';
        return;
    }
    size_t i = 0u;
    while (name[i] != '\0' && i + 1u < BLU2USB_UX_MOUSE_NAME_CAPACITY) {
        ux->current_mouse_name[i] = name[i];
        ++i;
    }
    ux->current_mouse_name[i] = '\0';
}


void blu2usb_ux_set_movement(blu2usb_ux_model_t *ux,
                             blu2usb_movement_settings_t settings)
{
    if (ux == NULL || !blu2usb_movement_valid(settings)) return;
    ux->movement_current = settings;
    if (ux->saved_connected_bond >= 0 &&
        (unsigned)ux->saved_connected_bond < BLU2USB_UX_MAX_SAVED_MICE)
        ux->saved_movements[ux->saved_connected_bond] = settings;
}

void blu2usb_ux_set_saved_movement(blu2usb_ux_model_t *ux, unsigned logical_bond,
                                   blu2usb_movement_settings_t settings)
{
    if (ux == NULL || logical_bond >= BLU2USB_UX_MAX_SAVED_MICE ||
        !blu2usb_movement_valid(settings)) return;
    ux->saved_movements[logical_bond] = settings;
}

static bool dpi_screen(blu2usb_screen_id_t screen)
{
    return screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI ||
           screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI ||
           screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI ||
           screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI;
}

static unsigned dpi_slot(blu2usb_screen_id_t screen)
{
    switch (screen) {
    case BLU2USB_SCREEN_HORIZONTAL_FAST_DPI: return 1u;
    case BLU2USB_SCREEN_VERTICAL_SLOW_DPI: return 2u;
    case BLU2USB_SCREEN_VERTICAL_FAST_DPI: return 3u;
    default: return 0u;
    }
}

static uint8_t gain_from_selection(bool fast, unsigned selection)
{
    static const uint8_t slow[4] = {4,3,2,1};
    static const uint8_t fast_gains[4] = {5,6,8,12};
    return (fast ? fast_gains : slow)[selection % 4u];
}

static unsigned selection_from_gain(uint8_t gain, bool fast)
{
    for (unsigned i = 0u; i < 4u; ++i)
        if (gain_from_selection(fast, i) == gain) return i;
    return 0u;
}

void blu2usb_ux_set_custom_target(blu2usb_ux_model_t *ux, blu2usb_mouse_source_t source, blu2usb_mouse_target_t target) {
    if ((unsigned)source < BLU2USB_MOUSE_SOURCE_COUNT && (unsigned)target < BLU2USB_MOUSE_TARGET_COUNT) {
        if (ux->custom_targets[source] != target) ux->custom_dirty = true;
        ux->custom_targets[source] = target;
        /* The menu order differs from persisted target numeric IDs. */
        if (is_will_become(ux->screen) && ux->custom_source == source) {
            switch (target) {
            case BLU2USB_MOUSE_TARGET_LEFT: ux->selection = 0u; break;
            case BLU2USB_MOUSE_TARGET_RIGHT: ux->selection = 1u; break;
            case BLU2USB_MOUSE_TARGET_MIDDLE: ux->selection = 2u; break;
            case BLU2USB_MOUSE_TARGET_FORWARD: ux->selection = 3u; break;
            case BLU2USB_MOUSE_TARGET_BACKWARD: ux->selection = 4u; break;
            case BLU2USB_MOUSE_TARGET_MODIFIER: ux->selection = 5u; break;
            default: ux->selection = 0u; break;
            }
        }
    }
}

const blu2usb_screen_template_t *blu2usb_ux_screen_template(blu2usb_screen_id_t screen) {
    if ((unsigned)screen >= BLU2USB_SCREEN_COUNT) return 0;
    return &screens[screen];
}

static blu2usb_mouse_target_t custom_target_for_selection(unsigned selection) {
    static const blu2usb_mouse_target_t targets[6] = {
        BLU2USB_MOUSE_TARGET_LEFT,
        BLU2USB_MOUSE_TARGET_RIGHT,
        BLU2USB_MOUSE_TARGET_MIDDLE,
        BLU2USB_MOUSE_TARGET_FORWARD,
        BLU2USB_MOUSE_TARGET_BACKWARD,
        BLU2USB_MOUSE_TARGET_MODIFIER,
    };
    return selection < 6u ?
        targets[selection] : BLU2USB_MOUSE_TARGET_LEFT;
}

static unsigned custom_selection_for_target(blu2usb_mouse_target_t target) {
    switch (target) {
    case BLU2USB_MOUSE_TARGET_LEFT: return 0u;
    case BLU2USB_MOUSE_TARGET_RIGHT: return 1u;
    case BLU2USB_MOUSE_TARGET_MIDDLE: return 2u;
    case BLU2USB_MOUSE_TARGET_FORWARD: return 3u;
    case BLU2USB_MOUSE_TARGET_BACKWARD: return 4u;
    case BLU2USB_MOUSE_TARGET_MODIFIER: return 5u;
    default: return 0u;
    }
}

static blu2usb_screen_id_t custom_screen_for(unsigned selection) {
    static const blu2usb_screen_id_t ids[5] = {
        BLU2USB_SCREEN_LEFT_WILL_BECOME, BLU2USB_SCREEN_RIGHT_WILL_BECOME,
        BLU2USB_SCREEN_MIDDLE_WILL_BECOME, BLU2USB_SCREEN_FORWARD_WILL_BECOME,
        BLU2USB_SCREEN_BACKWARD_WILL_BECOME
    };
    return ids[selection < 5 ? selection : 0];
}

void blu2usb_ux_custom_target_committed(blu2usb_ux_model_t *ux,
    blu2usb_mouse_source_t source,
    const blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT])
{
    if (ux == NULL || targets == NULL || (unsigned)source >= BLU2USB_MOUSE_SOURCE_COUNT)
        return;
    memcpy(ux->custom_targets, targets, sizeof(ux->custom_targets));
    ux->custom_dirty = true;
    enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
    ux->selection = (unsigned)source;
}

static blu2usb_mouse_source_t custom_source_for(unsigned selection) {
    static const blu2usb_mouse_source_t ids[5] = {
        BLU2USB_MOUSE_SOURCE_LEFT, BLU2USB_MOUSE_SOURCE_RIGHT, BLU2USB_MOUSE_SOURCE_MIDDLE,
        BLU2USB_MOUSE_SOURCE_FORWARD, BLU2USB_MOUSE_SOURCE_BACKWARD
    };
    return ids[selection < 5 ? selection : 0];
}

blu2usb_ux_command_t blu2usb_ux_input(blu2usb_ux_model_t *ux, blu2usb_control_t control, bool pressed) {
    blu2usb_ux_command_t cmd = no_command();
    blu2usb_interaction_event_t event = blu2usb_interaction_input(&ux->interaction, control, pressed);
    if (event.kind == BLU2USB_INTERACTION_NONE) return cmd;
    if (event.kind == BLU2USB_INTERACTION_UNLOCK) {
        enter(ux, BLU2USB_SCREEN_HOME);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING_HELP) {
        /* Mouse UI v1: Help cancels saved search; Any Key returns to
         * HOME RETRY and the interaction is consumed. */
        ux->screen = BLU2USB_SCREEN_HOME_RETRY;
        ux->selection = 0u;
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HELP_HOME_CONNECTED) {
        const blu2usb_screen_id_t target = ux->return_screen;
        const unsigned selection = ux->return_selection;
        enter(ux, target);
        if (ux->screen == BLU2USB_SCREEN_HOME)
            ux->selection = selection % 4u;
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS) {
        const unsigned selection = ux->return_selection;
        enter(ux, BLU2USB_SCREEN_MOUSE_OPTIONS);
        ux->selection = selection % 3u;
        return cmd;
    }

    if (is_help(ux->screen)) {
        const unsigned selection = ux->return_selection;
        const blu2usb_screen_id_t previous = ux->return_screen;
        enter(ux, previous);
        if (dpi_screen(previous) || previous == BLU2USB_SCREEN_ROTATION)
            ux->selection = selection % 4u;
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_SEARCHING_FIRST) {
        /* HOPE-01 replaces the legacy LEARN THE KEYS presentation in-place.
         * All controls are didactic only on SEARCHING FIRST MOUSE. */
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_MOUSE_SAVED) {
        /* HOPE-02 replaces the legacy success presentation in-place.
         * Only KEY Y has an action: lock on release. All other controls,
         * including KEY B, are didactic and do not navigate. */
        if (control == BLU2USB_CONTROL_KEY_Y)
            blu2usb_interaction_lock(&ux->interaction);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING &&
        control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = BLU2USB_SCREEN_HOME_SEARCHING;
        enter(ux, BLU2USB_SCREEN_HOME_SEARCHING_HELP);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME_RETRY &&
        control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = BLU2USB_SCREEN_HOME_RETRY;
        enter(ux, BLU2USB_SCREEN_HOME_RETRY_HELP);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME_RETRY &&
        control == BLU2USB_CONTROL_KEY_A) {
        enter(ux, BLU2USB_SCREEN_HOME_SEARCHING);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME_RETRY &&
        control == BLU2USB_CONTROL_KEY_B) {
        /* KEY B is not an action on Mouse UI v1 HOME RETRY. */
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_PAIR_MOUSE &&
        control == BLU2USB_CONTROL_KEY_X) {
        /* Opening Pair New Help cancels the active Pair New transaction.
         * Mouse UI v1 returns from that Help into retry-pair-new. */
        ux->return_screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;
        enter(ux, BLU2USB_SCREEN_HELP_PAIR_NEW);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_PAIR_MOUSE &&
        control == BLU2USB_CONTROL_KEY_B) {
        enter(ux, BLU2USB_SCREEN_HOME);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_RETRY_PAIR_NEW &&
        control == BLU2USB_CONTROL_KEY_A) {
        enter(ux, BLU2USB_SCREEN_PAIR_MOUSE);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_RETRY_PAIR_NEW &&
        control == BLU2USB_CONTROL_KEY_B) {
        enter(ux, BLU2USB_SCREEN_HOME);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_RETRY_PAIR_NEW &&
        control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = BLU2USB_SCREEN_RETRY_PAIR_NEW;
        enter(ux, BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&
        control == BLU2USB_CONTROL_KEY_Y &&
        !ux->remove_pending) {
        ux->return_screen = BLU2USB_SCREEN_REMOVE_THIS;
        ux->return_selection = ux->selection;
        enter(ux, BLU2USB_SCREEN_HELP_REMOVE_THIS);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&
        control == BLU2USB_CONTROL_KEY_Y && ux->remove_pending) {
        /* Pending removal disables Help and must not invoke global Lock. */
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&
        control == BLU2USB_CONTROL_KEY_B) {
        if (!ux->remove_pending) {
            const int page = blu2usb_ux_saved_page_for_bond(
                ux, ux->remove_target_bond);
            enter(ux, BLU2USB_SCREEN_SAVED_DEVICES);
            if (page >= 0) ux->saved_page = (unsigned)page;
            ux->remove_target_bond = -1;
        }
        return cmd;
    }

    if (control == BLU2USB_CONTROL_KEY_Y && lock_allowed(ux->screen)) {
        blu2usb_interaction_lock(&ux->interaction);
        return cmd;
    }

    if (control == BLU2USB_CONTROL_KEY_B) {
        if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING) {
            ux->screen = BLU2USB_SCREEN_HOME_RETRY;
            ux->selection = 0u;
        } else if (blu2usb_ux_is_shortcuts_screen(ux->screen)) {
            ux->screen = (blu2usb_screen_id_t)(BLU2USB_SCREEN_COMBINATIONS_1 + ux->editing_combination / 2u);
            ux->selection = ux->editing_combination % 2u;
        } else if (blu2usb_ux_is_combinations_screen(ux->screen)) {
            enter(ux, BLU2USB_SCREEN_MOUSE_OPTIONS);
            ux->selection = 2u; /* Restore MODIFIER SHORTCUTS entry focus. */
        } else if (is_will_become(ux->screen)) {
            const unsigned source = (unsigned)ux->custom_source;
            enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
            ux->selection = source < BLU2USB_MOUSE_SOURCE_COUNT ? source : 0u;
        } else if (ux->screen != BLU2USB_SCREEN_HOME) {
            enter(ux, back_target(ux));
        }
        return cmd;
    }

    if (blu2usb_ux_is_combinations_screen(ux->screen)) {
        if (control == BLU2USB_CONTROL_JOY_LEFT ||
            control == BLU2USB_CONTROL_JOY_RIGHT) {
            const unsigned page = (unsigned)(ux->screen - BLU2USB_SCREEN_COMBINATIONS_1);
            const unsigned next = control == BLU2USB_CONTROL_JOY_LEFT
                ? wrap_prev(page, BLU2USB_COMBINATIONS_PAGE_COUNT)
                : wrap_next(page, BLU2USB_COMBINATIONS_PAGE_COUNT);
            /* Keep the same logical option selected when paging. */
            ux->screen = (blu2usb_screen_id_t)(BLU2USB_SCREEN_COMBINATIONS_1 + next);
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_A) {
            const blu2usb_modifier_combo_id_t combo = blu2usb_ux_selected_combination(ux);
            if (combo < BLU2USB_MOD_COMBO_COUNT &&
                !blu2usb_ux_combination_unavailable(ux, (unsigned)combo) &&
                ux->shortcut_assignments[combo] != 0u) {
                cmd.kind = BLU2USB_UX_COMMAND_RESET_SHORTCUT;
                cmd.combination_id = (uint8_t)combo;
            }
            return cmd;
        }
        if (control == BLU2USB_CONTROL_JOY_PRESS) {
            const blu2usb_modifier_combo_id_t combo = blu2usb_ux_selected_combination(ux);
            if (combo >= BLU2USB_MOD_COMBO_COUNT ||
                blu2usb_ux_combination_unavailable(ux, (unsigned)combo))
                return cmd;
            ux->editing_combination = (uint8_t)combo;
            const uint8_t assigned = ux->shortcut_assignments[combo];
            const uint8_t focus_id = assigned == 0u ? 1u : assigned;
            ux->screen = (blu2usb_screen_id_t)(
                BLU2USB_SCREEN_SHORTCUTS_1 + (focus_id - 1u) / BLU2USB_SHORTCUTS_PER_PAGE);
            ux->selection = (focus_id - 1u) % BLU2USB_SHORTCUTS_PER_PAGE;
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_X) return cmd;
    }

    if (blu2usb_ux_is_shortcuts_screen(ux->screen)) {
        if (control == BLU2USB_CONTROL_JOY_LEFT ||
            control == BLU2USB_CONTROL_JOY_RIGHT) {
            const unsigned page = (unsigned)(ux->screen - BLU2USB_SCREEN_SHORTCUTS_1);
            const unsigned next = control == BLU2USB_CONTROL_JOY_LEFT
                ? wrap_prev(page, BLU2USB_SHORTCUTS_PAGE_COUNT)
                : wrap_next(page, BLU2USB_SHORTCUTS_PAGE_COUNT);
            const uint8_t assigned = ux->shortcut_assignments[ux->editing_combination];
            ux->screen = (blu2usb_screen_id_t)(BLU2USB_SCREEN_SHORTCUTS_1 + next);
            if (assigned > 0u &&
                (assigned - 1u) / BLU2USB_SHORTCUTS_PER_PAGE == next)
                ux->selection = (assigned - 1u) % BLU2USB_SHORTCUTS_PER_PAGE;
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_A) {
            cmd.kind = BLU2USB_UX_COMMAND_SET_SHORTCUT;
            cmd.combination_id = ux->editing_combination;
            cmd.shortcut_id = blu2usb_ux_selected_shortcut(ux);
            return cmd; /* commit-first: remain in editor until flash verifies */
        }
        if (control == BLU2USB_CONTROL_JOY_PRESS ||
            control == BLU2USB_CONTROL_KEY_X) return cmd;
    }

    unsigned count = blu2usb_ux_option_count(ux);
    if (count && control == BLU2USB_CONTROL_JOY_UP) { ux->selection = wrap_prev(ux->selection, count); return cmd; }
    if (count && control == BLU2USB_CONTROL_JOY_DOWN) { ux->selection = wrap_next(ux->selection, count); return cmd; }

    if (ux->screen == BLU2USB_SCREEN_HOME &&
        control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = BLU2USB_SCREEN_HOME;
        ux->return_selection = ux->selection;
        enter(ux, BLU2USB_SCREEN_HELP_HOME_CONNECTED);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_HOME && control == BLU2USB_CONTROL_JOY_PRESS) {
        static const blu2usb_screen_id_t dest[4] = {
            BLU2USB_SCREEN_MOUSE_OPTIONS,
            BLU2USB_SCREEN_MOVEMENT_OPTIONS,
            BLU2USB_SCREEN_SAVED_DEVICES,
            BLU2USB_SCREEN_PAIR_MOUSE
        };
        enter(ux, dest[ux->selection]);
        return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING && control == BLU2USB_CONTROL_JOY_PRESS) {
        static const blu2usb_screen_id_t dest[2] = {
            BLU2USB_SCREEN_SAVED_DEVICES,
            BLU2USB_SCREEN_PAIR_MOUSE
        };
        enter(ux, dest[ux->selection]);
        return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_HOME_RETRY && control == BLU2USB_CONTROL_JOY_PRESS) {
        static const blu2usb_screen_id_t dest[2] = {
            BLU2USB_SCREEN_SAVED_DEVICES,
            BLU2USB_SCREEN_PAIR_MOUSE
        };
        enter(ux, dest[ux->selection]);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS &&
        control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = BLU2USB_SCREEN_MOUSE_OPTIONS;
        ux->return_selection = ux->selection;
        enter(ux, BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS &&
        control == BLU2USB_CONTROL_JOY_LEFT) {
        enter(ux, BLU2USB_SCREEN_HOME);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS && control == BLU2USB_CONTROL_JOY_PRESS) {
        const unsigned selected = ux->selection;
        switch (selected) {
        case 0:
            enter(ux, ux->active_profile == BLU2USB_MOUSE_PROFILE_PASSTHROUGH
                      ? BLU2USB_SCREEN_PASSTHROUGH_APPLIED
                      : BLU2USB_SCREEN_APPLY_PASSTHROUGH);
            break;
        case 1:
            if (ux->active_profile != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP)
                ux->custom_dirty = true;
            enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
            break;
        case 2:
            enter(ux, BLU2USB_SCREEN_COMBINATIONS_1);
            break;
        default:
            break;
        }
        return cmd;
    }


    if (ux->screen == BLU2USB_SCREEN_MOVEMENT_OPTIONS) {
        if (control == BLU2USB_CONTROL_KEY_A) {
            cmd.kind = BLU2USB_UX_COMMAND_APPLY_MOVEMENT;
            cmd.movement = blu2usb_movement_defaults();
        } else if (control == BLU2USB_CONTROL_JOY_PRESS) {
            const unsigned axis = ux->selection;
            if (axis == 2u) {
                enter(ux, BLU2USB_SCREEN_ROTATION);
                ux->selection = ux->movement_current.rotation_quadrants;
            } else {
                const uint8_t gain = axis == 0u ?
                    ux->movement_current.horizontal_quarters :
                    ux->movement_current.vertical_quarters;
                const bool fast = gain > 4u;
                const blu2usb_screen_id_t target = axis == 0u ?
                    (fast ? BLU2USB_SCREEN_HORIZONTAL_FAST_DPI : BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI) :
                    (fast ? BLU2USB_SCREEN_VERTICAL_FAST_DPI : BLU2USB_SCREEN_VERTICAL_SLOW_DPI);
                enter(ux, target);
                ux->selection = selection_from_gain(gain, fast);
                ux->movement_dpi_selections[dpi_slot(target)] = (uint8_t)ux->selection;
            }
        }
        return cmd;
    }

    if (dpi_screen(ux->screen) || ux->screen == BLU2USB_SCREEN_ROTATION) {
        const bool dpi = dpi_screen(ux->screen);
        const bool fast = ux->screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI ||
                          ux->screen == BLU2USB_SCREEN_VERTICAL_FAST_DPI;
        if (dpi && (control == BLU2USB_CONTROL_JOY_LEFT ||
                    control == BLU2USB_CONTROL_JOY_RIGHT)) {
            ux->movement_dpi_selections[dpi_slot(ux->screen)] = (uint8_t)ux->selection;
            const blu2usb_screen_id_t dest =
                ux->screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI ? BLU2USB_SCREEN_HORIZONTAL_FAST_DPI :
                ux->screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI ? BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI :
                ux->screen == BLU2USB_SCREEN_VERTICAL_SLOW_DPI ? BLU2USB_SCREEN_VERTICAL_FAST_DPI :
                BLU2USB_SCREEN_VERTICAL_SLOW_DPI;
            enter(ux, dest);
            ux->selection = ux->movement_dpi_selections[dpi_slot(dest)];
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_X) {
            ux->return_screen = ux->screen;
            ux->return_selection = ux->selection;
            enter(ux, !dpi ? BLU2USB_SCREEN_HELP_ROTATION :
                      fast ? BLU2USB_SCREEN_HELP_FAST_DPI :
                      BLU2USB_SCREEN_HELP_SLOW_DPI);
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_A) {
            cmd.kind = BLU2USB_UX_COMMAND_APPLY_MOVEMENT;
            cmd.movement = ux->movement_current;
            if (!dpi) cmd.movement.rotation_quadrants = (uint8_t)ux->selection;
            else if (ux->screen == BLU2USB_SCREEN_HORIZONTAL_SLOW_DPI ||
                     ux->screen == BLU2USB_SCREEN_HORIZONTAL_FAST_DPI)
                cmd.movement.horizontal_quarters = gain_from_selection(fast, ux->selection);
            else
                cmd.movement.vertical_quarters = gain_from_selection(fast, ux->selection);
            return cmd;
        }
    }

    if (ux->screen == BLU2USB_SCREEN_EDIT_CUSTOM) {
        if (control == BLU2USB_CONTROL_JOY_PRESS) {
            unsigned selected = ux->selection;
            ux->custom_source = custom_source_for(selected);
            enter(ux, custom_screen_for(selected));
            ux->selection =
                custom_selection_for_target(ux->custom_targets[ux->custom_source]);
            return cmd;
        }
        if (control == BLU2USB_CONTROL_KEY_A) {
            cmd.kind = BLU2USB_UX_COMMAND_APPLY_CUSTOM;
            return cmd;
        }
    }

    if (is_will_become(ux->screen) &&
        (control == BLU2USB_CONTROL_KEY_A ||
         control == BLU2USB_CONTROL_JOY_PRESS)) {
        const unsigned source = (unsigned)ux->custom_source;
        cmd.kind = BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET;
        cmd.source = ux->custom_source;
        cmd.target = custom_target_for_selection(ux->selection);
        /* Stay in WILL BECOME until the persisted draft write succeeds;
         * the command handler calls blu2usb_ux_custom_target_committed. */
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_SAVED_DEVICES &&
        (control == BLU2USB_CONTROL_JOY_LEFT || control == BLU2USB_CONTROL_JOY_RIGHT) && ux->saved_pages) {
        ux->saved_page = control == BLU2USB_CONTROL_JOY_LEFT ? wrap_prev(ux->saved_page, ux->saved_pages) : wrap_next(ux->saved_page, ux->saved_pages);
        ux->selection = 0;
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_SAVED_DEVICES &&
        control == BLU2USB_CONTROL_JOY_PRESS &&
        ux->saved_device_count > 0u) {
        ux->remove_pending = false;
        ux->remove_target_bond =
            blu2usb_ux_saved_bond_for_page(ux, ux->saved_page);
        if (ux->remove_target_bond >= 0)
            enter(ux, BLU2USB_SCREEN_REMOVE_THIS);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&
        control == BLU2USB_CONTROL_KEY_A) {
        if (ux->remove_pending) return cmd;
        if (ux->remove_target_bond >= 0) {
            ux->remove_pending = true;
            cmd.kind = BLU2USB_UX_COMMAND_REMOVE_MOUSE;
            cmd.saved_bond = ux->remove_target_bond;
        }
        return cmd;
    }


    if (ux->screen == BLU2USB_SCREEN_APPLY_PASSTHROUGH && control == BLU2USB_CONTROL_KEY_A) {
        cmd.kind = BLU2USB_UX_COMMAND_APPLY_PASSTHROUGH;
    }

    return cmd;
}

uint16_t blu2usb_ux_learn_white_span_mask(const blu2usb_ux_model_t *ux) {
    if (ux->screen != BLU2USB_SCREEN_SEARCHING_FIRST) return 0;
    uint16_t mask = 0;
    for (unsigned c = 0; c < BLU2USB_CONTROL_COUNT; ++c) {
        if (blu2usb_interaction_is_pressed(&ux->interaction, (blu2usb_control_t)c)) {
            if (c == BLU2USB_CONTROL_KEY_Y) mask |= (uint16_t)(1u << 9) | (uint16_t)(1u << 10) | (uint16_t)(1u << 11);
            else mask |= (uint16_t)(1u << c);
        }
    }
    return mask;
}