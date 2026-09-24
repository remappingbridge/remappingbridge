#include "blu2usb/ux_model/ux_model.h"

#define DYN(row) ((uint16_t)(1u << (row)))
#define EMPTY ""

static const blu2usb_screen_template_t screens[BLU2USB_SCREEN_COUNT] = {
    [BLU2USB_SCREEN_HOME] = {{"UNKNOWN MOUSE"," NO REMAP PASSTHROUGH"," SAVED DEVICES"," PAIR NEW MOUSE"," LEARN THE KEYS",EMPTY,"JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP TO REMOVE"},DYN(0)|DYN(1)},
    [BLU2USB_SCREEN_HELP_HOME_CONNECTED] = {{"REMOVE CONNECTED HELP","TO DISCONNECT THE","CURRENTLY CONNECTED","MOUSE NAVIGATE TO:","STEP 1. SAVED DEVICES","STEP 2. REMOVE DEVICE","STEP 3. KEY A: REMOVE",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HOME_SEARCHING] = {{"SEARCHING SAVED MOUSE"," SAVED DEVICES"," PAIR NEW MOUSE"," LEARN THE KEYS",EMPTY,"KEY B: CANCEL SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HOME_SEARCHING_HELP] = {{"HOME SEARCHING HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_HOME_RETRY] = {{"DEVICE NOT FOUND"," SAVED DEVICES"," PAIR NEW MOUSE"," LEARN THE KEYS",EMPTY,"KEY A: RETRY SEARCH","JOY UP / DOWN: SELECT","JOY PRESS: ACCESS","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HOME_RETRY_HELP] = {{"HOME RETRY HELP","THE MATCHING ATTEMPT","TOOK PLACE ONLY FOR","DEVICES ALREADY SAVED","IN THE PREFERENCES,","BUT NOT FOR DEVICES","THAT WERE NOT SAVED.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOUSE_STATUS] = {{"MOUSE STATUS","MOUSE NOT CONNECTED","PROFILE: PASSTHROUGH","FWD: AUTO HIDPP","BACK: AUTO STD",EMPTY,"JOY RIGHT\\LEFT: PAGE","KEY B: BACK","KEY X: MOUSE HELP"},DYN(1)|DYN(2)|DYN(3)|DYN(4)},
    [BLU2USB_SCREEN_OTHER_DEVICES_STATUS] = {{"OTHER DEVICES STATUS","KEYBOARD","CONNECTED","COMPOSITE","NOT CONNECTED",EMPTY,"JOY RIGHT\\LEFT: PAGE","KEY B: BACK","KEY X: DEVICES HELP"},DYN(1)|DYN(2)|DYN(3)|DYN(4)},
    [BLU2USB_SCREEN_MOUSE_HELP] = {{"MOUSE HELP",EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,"ANY KEY: BACK"},0x00fe},
    [BLU2USB_SCREEN_DEVICES_HELP] = {{"DEVICES HELP","KEYBOARD IS DIFFERENT","FROM COMPOSITE.","COMPOSITE IS TOUCHPAD","AND KEYBOARD EMBEDDED","TOGETHER AND IT PAIRS","ITS OWN BLUETOOTH.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOUSE_OPTIONS] = {{"REMAPPING OPTIONS"," PASSTHROUGH"," STANDARD REMAP"," ESCAPE REMAP"," CUSTOM REMAP",EMPTY,"JOY PRESS: ACCESS","KEY B: BACK","KEY X: HELP"},0},
    [BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS] = {{"REMAPPER OPTIONS HELP","CHOOSE FROM THE","OPTIONS TO CHANGE THE","FUNCTIONS OF THE","MOUSE BUTTONS.","PASSTHROUGH IS THE","DEFAULT OPTION.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_PAIR_MOUSE] = {{"PAIR NEW MOUSE","TRYING TO CONNECT","A NEW MOUSE THAT","IS NOT LISTED","IN SAVED DEVICES",EMPTY,"KEY B: CANCEL","KEY X: HELP","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_HELP_PAIR_NEW] = {{"PAIR NEW DEVICE HELP","TO CONNECT A SAVED","DEVICE FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_RETRY_PAIR_NEW] = {{"NEW MOUSE NOT FOUND","NO NEW MOUSE OUTSIDE","THE LIST OF SAVED","DEVICES WAS FOUND",EMPTY,"KEY A: RETRY NEW PAIR","KEY B: BACK TRY SAVED","KEY X: HELP","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_HELP_RETRY_PAIR_NEW] = {{"MOUSE NOT FOUND HELP","TO CONNECT A SAVED","DEVICE FIRST UNPLUG","CURRENTLY CONNECTED","MOUSE AND PRESS THE","KEY B TO BACK UNTIL","SEARCHING APPEARS.",EMPTY,"ANY KEY: BACK"},0},
    [BLU2USB_SCREEN_MOUSE_SAVED] = {{"FIRST MOUSE CONNECTED","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y",EMPTY," KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_APPLY_PASSTHROUGH] = {{"APPLY PASSTHROUGH","ORIGINAL MOUSE","BUTTONS POSITION","ARE NOT ACTIVE",EMPTY,EMPTY,"KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_PASSTHROUGH_APPLIED] = {{"PASSTHROUGH ACTIVE","ORIGINAL MOUSE","BUTTONS POSITION","ARE ACTIVE NOW",EMPTY,EMPTY,EMPTY,"KEY B: BACK","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_APPLY_DEFAULT] = {{"APPLY STANDARD REMAP","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD",EMPTY,"KEY A: APPLY","KEY B: CANCEL","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_DEFAULT_APPLIED] = {{"STANDARD REMAP ACTIVE","FORWARD IS LEFT","LEFT IS FORWARD","BACKWARD IS RIGHT","RIGHT IS BACKWARD",EMPTY,EMPTY,"KEY B: BACK","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_APPLY_ESCAPE] = {{"APPLY ESCAPE REMAP","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD",EMPTY,"KEY A: APPLY","KEY B: CANCEL"},0},
    [BLU2USB_SCREEN_ESCAPE_APPLIED] = {{"ESCAPE APPLIED ACTIVE","FORWARD IS LEFT","BACKWARD IS RIGHT","LEFT IS ESCAPE","RIGHT IS BACKWARD","MIDDLE IS FORWARD",EMPTY,"KEY B: BACK","KEY Y: LOCK"},0},
    [BLU2USB_SCREEN_EDIT_CUSTOM] = {{"EDIT CUSTOM REMAP"," LEFT IS LEFT"," RIGHT IS RIGHT"," MIDDLE IS MIDDLE"," FORWARD IS FORWARD"," BACKWARD IS BACKWARD",EMPTY,"JOY PRESS: ACCESS","KEY A: APPLY CUSTOM"},DYN(1)|DYN(2)|DYN(3)|DYN(4)|DYN(5)},
    [BLU2USB_SCREEN_CUSTOM_APPLIED] = {{"EDIT CUSTOM REMAP"," LEFT IS LEFT"," RIGHT IS RIGHT"," MIDDLE IS MIDDLE"," FORWARD IS FORWARD"," BACKWARD IS BACKWARD",EMPTY,"JOY PRESS: ACCESS","KEY A: APPLY CUSTOM"},DYN(1)|DYN(2)|DYN(3)|DYN(4)|DYN(5)},
    [BLU2USB_SCREEN_LEFT_WILL_BECOME] = {{"LEFT WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_RIGHT_WILL_BECOME] = {{"RIGHT WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_MIDDLE_WILL_BECOME] = {{"MIDDLE WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_FORWARD_WILL_BECOME] = {{"FORWARD WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_BACKWARD_WILL_BECOME] = {{"BACKWARD WILL BECOME"," LEFT"," RIGHT"," MIDDLE"," ESCAPE"," FORWARD"," BACKWARD",EMPTY,"KEY A: APPLY AND BACK"},0},
    [BLU2USB_SCREEN_SAVED_DEVICES] = {{"0 OF 0","UNKNOWN MOUSE","STATUS: DISCONNECTED","PROFILE: PASSTHROUGH"," REMOVE DEVICE",EMPTY,"JOY RIGHT\\LEFT: PAGE","JOY PRESS: ACCESS","KEY B: BACK"},DYN(0)|DYN(1)|DYN(2)|DYN(3)},
    [BLU2USB_SCREEN_REMOVE_THIS] = {{"REMOVE THIS MOUSE","UNKNOWN MOUSE",EMPTY,"PAIRING AND MAPPINGS","WILL BE DELETED",EMPTY,"KEY A: REMOVE","KEY B: CANCEL","KEY X: HELP"},DYN(1)},
    [BLU2USB_SCREEN_LEARN_KEYS] = {{"SEARCHING FIRST MOUSE","PRESS TO LEARN KEYS","WHILE WAIT CONNECTION","       JOY UP","  JOY    JOY    JOY","  LEFT  PRESS  RIGHT","      JOY DOWN"," KEY A         KEY X"," KEY B         KEY Y"},0},
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
           screen == BLU2USB_SCREEN_MOUSE_HELP || screen == BLU2USB_SCREEN_DEVICES_HELP;
}

static bool is_will_become(blu2usb_screen_id_t screen) {
    return screen >= BLU2USB_SCREEN_LEFT_WILL_BECOME && screen <= BLU2USB_SCREEN_BACKWARD_WILL_BECOME;
}

static bool lock_allowed(blu2usb_screen_id_t screen) {
    return !is_help(screen) && screen != BLU2USB_SCREEN_LEARN_KEYS;
}

static void enter(blu2usb_ux_model_t *ux, blu2usb_screen_id_t screen) {
    if (screen == BLU2USB_SCREEN_HOME) {
        if (blu2usb_ux_mouse_connected()) screen = BLU2USB_SCREEN_HOME;
        else if (ux->saved_device_count > 0u) screen = BLU2USB_SCREEN_HOME_SEARCHING;
        else screen = BLU2USB_SCREEN_LEARN_KEYS;
    }
    ux->screen = screen;
    ux->selection = 0;
}

static unsigned wrap_prev(unsigned value, unsigned count) { return count ? (value + count - 1u) % count : 0u; }
static unsigned wrap_next(unsigned value, unsigned count) { return count ? (value + 1u) % count : 0u; }

static blu2usb_screen_id_t back_target(const blu2usb_ux_model_t *ux) {
    switch (ux->screen) {
    case BLU2USB_SCREEN_HOME: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_MOUSE_STATUS:
    case BLU2USB_SCREEN_OTHER_DEVICES_STATUS: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_MOUSE_HELP:
    case BLU2USB_SCREEN_DEVICES_HELP:
    case BLU2USB_SCREEN_HELP_REMAPPER_OPTIONS: return ux->return_screen;
    case BLU2USB_SCREEN_MOUSE_OPTIONS: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_PAIR_MOUSE:
    case BLU2USB_SCREEN_MOUSE_SAVED: return BLU2USB_SCREEN_MOUSE_OPTIONS;
    case BLU2USB_SCREEN_APPLY_PASSTHROUGH:
    case BLU2USB_SCREEN_APPLY_DEFAULT:
    case BLU2USB_SCREEN_APPLY_ESCAPE:
    case BLU2USB_SCREEN_EDIT_CUSTOM:
    case BLU2USB_SCREEN_CUSTOM_APPLIED:
    case BLU2USB_SCREEN_PASSTHROUGH_APPLIED:
    case BLU2USB_SCREEN_DEFAULT_APPLIED:
    case BLU2USB_SCREEN_ESCAPE_APPLIED: return BLU2USB_SCREEN_MOUSE_OPTIONS;
    case BLU2USB_SCREEN_LEFT_WILL_BECOME:
    case BLU2USB_SCREEN_RIGHT_WILL_BECOME:
    case BLU2USB_SCREEN_MIDDLE_WILL_BECOME:
    case BLU2USB_SCREEN_FORWARD_WILL_BECOME:
    case BLU2USB_SCREEN_BACKWARD_WILL_BECOME: return BLU2USB_SCREEN_EDIT_CUSTOM;
    case BLU2USB_SCREEN_SAVED_DEVICES: return BLU2USB_SCREEN_HOME;
    case BLU2USB_SCREEN_REMOVE_THIS: return BLU2USB_SCREEN_SAVED_DEVICES;
    case BLU2USB_SCREEN_LEARN_KEYS: return BLU2USB_SCREEN_LEARN_KEYS;
    default: return BLU2USB_SCREEN_HOME;
    }
}

unsigned blu2usb_ux_option_count(const blu2usb_ux_model_t *ux) {
    switch (ux->screen) {
    case BLU2USB_SCREEN_HOME: return 4;
    case BLU2USB_SCREEN_HOME_SEARCHING:
    case BLU2USB_SCREEN_HOME_RETRY: return 3;
    case BLU2USB_SCREEN_MOUSE_OPTIONS: return 4;
    case BLU2USB_SCREEN_EDIT_CUSTOM: return 5;
    case BLU2USB_SCREEN_LEFT_WILL_BECOME:
    case BLU2USB_SCREEN_RIGHT_WILL_BECOME:
    case BLU2USB_SCREEN_MIDDLE_WILL_BECOME:
    case BLU2USB_SCREEN_FORWARD_WILL_BECOME:
    case BLU2USB_SCREEN_BACKWARD_WILL_BECOME: return BLU2USB_MOUSE_TARGET_COUNT;
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
    ux->status_page = 0;
    ux->saved_page = 0;
    ux->saved_pages = 1;
    ux->saved_device_count = 0;
    ux->saved_connected_bond = -1;
    ux->saved_front_bond = -1;
    memset(ux->saved_mouse_names, 0, sizeof(ux->saved_mouse_names));
    ux->current_mouse_name[0] = '\0';
    ux->active_profile = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    ux->custom_dirty = false;
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

void blu2usb_ux_set_custom_target(blu2usb_ux_model_t *ux, blu2usb_mouse_source_t source, blu2usb_mouse_target_t target) {
    if ((unsigned)source < BLU2USB_MOUSE_SOURCE_COUNT && (unsigned)target < BLU2USB_MOUSE_TARGET_COUNT) {
        if (ux->custom_targets[source] != target) ux->custom_dirty = true;
        ux->custom_targets[source] = target;
        if (is_will_become(ux->screen) && ux->custom_source == source) ux->selection = (unsigned)target;
    }
}

const blu2usb_screen_template_t *blu2usb_ux_screen_template(blu2usb_screen_id_t screen) {
    if ((unsigned)screen >= BLU2USB_SCREEN_COUNT) return 0;
    return &screens[screen];
}

static blu2usb_mouse_target_t custom_target_for_selection(unsigned selection) {
    static const blu2usb_mouse_target_t targets[BLU2USB_MOUSE_TARGET_COUNT] = {
        BLU2USB_MOUSE_TARGET_LEFT,
        BLU2USB_MOUSE_TARGET_RIGHT,
        BLU2USB_MOUSE_TARGET_MIDDLE,
        BLU2USB_MOUSE_TARGET_ESCAPE,
        BLU2USB_MOUSE_TARGET_FORWARD,
        BLU2USB_MOUSE_TARGET_BACKWARD,
    };
    return selection < BLU2USB_MOUSE_TARGET_COUNT ?
        targets[selection] : BLU2USB_MOUSE_TARGET_LEFT;
}

static unsigned custom_selection_for_target(blu2usb_mouse_target_t target) {
    switch (target) {
    case BLU2USB_MOUSE_TARGET_LEFT: return 0u;
    case BLU2USB_MOUSE_TARGET_RIGHT: return 1u;
    case BLU2USB_MOUSE_TARGET_MIDDLE: return 2u;
    case BLU2USB_MOUSE_TARGET_ESCAPE: return 3u;
    case BLU2USB_MOUSE_TARGET_FORWARD: return 4u;
    case BLU2USB_MOUSE_TARGET_BACKWARD: return 5u;
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
        ux->selection = selection % 4u;
        return cmd;
    }

    if (is_help(ux->screen)) {
        enter(ux, ux->return_screen);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_LEARN_KEYS) {
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

    if (control == BLU2USB_CONTROL_KEY_Y && lock_allowed(ux->screen)) {
        blu2usb_interaction_lock(&ux->interaction);
        return cmd;
    }

    if (control == BLU2USB_CONTROL_KEY_B) {
        if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING) {
            ux->screen = BLU2USB_SCREEN_HOME_RETRY;
            ux->selection = 0u;
        } else if (is_will_become(ux->screen)) {
            const unsigned source = (unsigned)ux->custom_source;
            enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
            ux->selection = source < BLU2USB_MOUSE_SOURCE_COUNT ? source : 0u;
        } else if (ux->screen != BLU2USB_SCREEN_HOME) {
            enter(ux, back_target(ux));
        }
        return cmd;
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
            BLU2USB_SCREEN_SAVED_DEVICES,
            BLU2USB_SCREEN_PAIR_MOUSE,
            BLU2USB_SCREEN_LEARN_KEYS
        };
        enter(ux, dest[ux->selection]);
        return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_HOME_SEARCHING && control == BLU2USB_CONTROL_JOY_PRESS) {
        static const blu2usb_screen_id_t dest[3] = {BLU2USB_SCREEN_SAVED_DEVICES, BLU2USB_SCREEN_PAIR_MOUSE, BLU2USB_SCREEN_LEARN_KEYS};
        enter(ux, dest[ux->selection]);
        return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_HOME_RETRY && control == BLU2USB_CONTROL_JOY_PRESS) {
        static const blu2usb_screen_id_t dest[3] = {BLU2USB_SCREEN_SAVED_DEVICES, BLU2USB_SCREEN_PAIR_MOUSE, BLU2USB_SCREEN_LEARN_KEYS};
        enter(ux, dest[ux->selection]);
        return cmd;
    }

    if ((ux->screen == BLU2USB_SCREEN_MOUSE_STATUS || ux->screen == BLU2USB_SCREEN_OTHER_DEVICES_STATUS) &&
        (control == BLU2USB_CONTROL_JOY_LEFT || control == BLU2USB_CONTROL_JOY_RIGHT)) {
        ux->status_page = ux->status_page ? 0u : 1u;
        enter(ux, ux->status_page ? BLU2USB_SCREEN_OTHER_DEVICES_STATUS : BLU2USB_SCREEN_MOUSE_STATUS);
        return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_MOUSE_STATUS && control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = ux->screen; enter(ux, BLU2USB_SCREEN_MOUSE_HELP); return cmd;
    }
    if (ux->screen == BLU2USB_SCREEN_OTHER_DEVICES_STATUS && control == BLU2USB_CONTROL_KEY_X) {
        ux->return_screen = ux->screen; enter(ux, BLU2USB_SCREEN_DEVICES_HELP); return cmd;
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
            enter(ux, ux->active_profile == BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP
                      ? BLU2USB_SCREEN_DEFAULT_APPLIED
                      : BLU2USB_SCREEN_APPLY_DEFAULT);
            break;
        case 2:
            enter(ux, ux->active_profile == BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP
                      ? BLU2USB_SCREEN_ESCAPE_APPLIED
                      : BLU2USB_SCREEN_APPLY_ESCAPE);
            break;
        case 3:
            if (ux->active_profile != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP)
                ux->custom_dirty = true;
            enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
            break;
        default:
            break;
        }
        return cmd;
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
        enter(ux, BLU2USB_SCREEN_EDIT_CUSTOM);
        ux->selection = source < BLU2USB_MOUSE_SOURCE_COUNT ? source : 0u;
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
        enter(ux, BLU2USB_SCREEN_REMOVE_THIS);
        return cmd;
    }

    if (ux->screen == BLU2USB_SCREEN_REMOVE_THIS &&
        control == BLU2USB_CONTROL_KEY_A) {
        const int bond = blu2usb_ux_saved_bond_for_page(ux, ux->saved_page);
        if (bond >= 0) {
            cmd.kind = BLU2USB_UX_COMMAND_REMOVE_MOUSE;
            cmd.saved_bond = bond;
        }
        return cmd;
    }

    /* HOPE-30 owns KEY X / help-remove-this. */

    if (ux->screen == BLU2USB_SCREEN_APPLY_PASSTHROUGH && control == BLU2USB_CONTROL_KEY_A) {
        cmd.kind = BLU2USB_UX_COMMAND_APPLY_PASSTHROUGH;
    } else if (ux->screen == BLU2USB_SCREEN_APPLY_DEFAULT && control == BLU2USB_CONTROL_KEY_A) {
        cmd.kind = BLU2USB_UX_COMMAND_APPLY_DEFAULT;
    } else if (ux->screen == BLU2USB_SCREEN_APPLY_ESCAPE && control == BLU2USB_CONTROL_KEY_A) {
        cmd.kind = BLU2USB_UX_COMMAND_APPLY_ESCAPE;
    }

    return cmd;
}

uint16_t blu2usb_ux_learn_white_span_mask(const blu2usb_ux_model_t *ux) {
    if (ux->screen != BLU2USB_SCREEN_LEARN_KEYS) return 0;
    uint16_t mask = 0;
    for (unsigned c = 0; c < BLU2USB_CONTROL_COUNT; ++c) {
        if (blu2usb_interaction_is_pressed(&ux->interaction, (blu2usb_control_t)c)) {
            if (c == BLU2USB_CONTROL_KEY_Y) mask |= (uint16_t)(1u << 9) | (uint16_t)(1u << 10) | (uint16_t)(1u << 11);
            else mask |= (uint16_t)(1u << c);
        }
    }
    return mask;
}