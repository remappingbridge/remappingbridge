/* MOD-10 release candidate: HOME-connected editor -> verified storage ->
 * reboot/masking. This is host integration, not hardware acceptance. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/profiles/profiles.h"
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/hid_aggregator/hid_aggregator.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t key)
{
    (void)blu2usb_ux_input(ux, key, true);
    return blu2usb_ux_input(ux, key, false);
}

static void enter_from_home(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux, 2u);
    assert(ux->screen == BLU2USB_SCREEN_HOME);
    (void)tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    (void)tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    (void)tap(ux, BLU2USB_CONTROL_JOY_DOWN);
    (void)tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen == BLU2USB_SCREEN_COMBINATIONS_1);
}

static void select_action(blu2usb_ux_model_t *ux, uint8_t id)
{
    assert(id >= 1u && id <= BLU2USB_SHORTCUT_COUNT);
    assert(ux->screen == BLU2USB_SCREEN_COMBINATIONS_1);
    assert(ux->selection == 0u);
    (void)tap(ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(blu2usb_ux_is_shortcuts_screen(ux->screen));
    const unsigned page = (id - 1u) / BLU2USB_SHORTCUTS_PER_PAGE;
    const unsigned selection = (id - 1u) % BLU2USB_SHORTCUTS_PER_PAGE;
    unsigned steps = 0u;
    while (ux->screen != (blu2usb_screen_id_t)(BLU2USB_SCREEN_SHORTCUTS_1 + page)) {
        (void)tap(ux, BLU2USB_CONTROL_JOY_RIGHT);
        assert(++steps <= BLU2USB_SHORTCUTS_PAGE_COUNT);
    }
    steps = 0u;
    while (ux->selection != selection) {
        (void)tap(ux, BLU2USB_CONTROL_JOY_DOWN);
        assert(++steps <= BLU2USB_SHORTCUTS_PER_PAGE);
    }
    assert(blu2usb_ux_selected_shortcut(ux) == id);
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(ux, &frame);
    blu2usb_ui_enforce_applied_visual_contract(ux, &frame);
    assert(frame.cells[0][0].character == 'S');
    assert(frame.cells[selection+1u][1].tone == BLU2USB_UI_TONE_EMPHASIZED);
    assert(blu2usb_ux_shortcut_name(id) != NULL);
}

static void save_commit(blu2usb_ux_model_t *ux, blu2usb_profiles_t *profiles,
                        blu2usb_ux_command_t intent)
{
    assert(intent.kind == BLU2USB_UX_COMMAND_SET_SHORTCUT);
    assert(intent.combination_id == 0u);
    const blu2usb_profiles_t original = *profiles;
    assert(blu2usb_profiles_set_shortcut(profiles, intent.combination_id,
                                        intent.shortcut_id));
    uint8_t encoded[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(profiles, encoded));
    bool migrated = true;
    blu2usb_profiles_t reread;
    assert(blu2usb_profiles_restore_any(&reread, encoded, sizeof(encoded), &migrated));
    assert(!migrated && reread.shortcuts[0] == intent.shortcut_id);
    for (unsigned slot=1;slot<BLU2USB_SHORTCUT_SLOT_COUNT;slot++)
        assert(reread.shortcuts[slot] == original.shortcuts[slot]);
    *profiles = reread;
    blu2usb_ux_shortcut_committed(ux, intent.combination_id, intent.shortcut_id);
    assert(ux->screen == BLU2USB_SCREEN_COMBINATIONS_1);
    assert(ux->shortcut_assignments[0] == intent.shortcut_id);
}

static void verify_entire_picker_and_commit(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_profiles_t profiles;
    blu2usb_profiles_init(&profiles);
    enter_from_home(&ux);
    for (uint8_t id=1u;id<=BLU2USB_SHORTCUT_COUNT;id++) {
        select_action(&ux,id);
        const blu2usb_ux_command_t intent = tap(&ux, BLU2USB_CONTROL_KEY_A);
        assert(intent.shortcut_id == id);
        assert(ux.screen >= BLU2USB_SCREEN_SHORTCUTS_1 &&
               ux.screen <= BLU2USB_SCREEN_SHORTCUTS_5);
        /* No UI commit until successful v3 serialization / verified reread. */
        assert(ux.shortcut_assignments[0] == (uint8_t)(id-1u));
        save_commit(&ux,&profiles,intent);
    }
    assert(profiles.shortcuts[0] == 30u);
    /* KEY B discards a pending draft without changing saved config. */
    (void)tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(blu2usb_ux_is_shortcuts_screen(ux.screen));
    (void)tap(&ux, BLU2USB_CONTROL_JOY_LEFT);
    (void)tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_COMBINATIONS_1);
    assert(ux.shortcut_assignments[0] == 30u);
    /* RESET targets only the selected combo and waits for flash success. */
    const blu2usb_ux_command_t reset = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(reset.kind == BLU2USB_UX_COMMAND_RESET_SHORTCUT);
    assert(reset.combination_id == 0u);
    assert(ux.shortcut_assignments[0] == 30u);
    assert(blu2usb_profiles_set_shortcut(&profiles,0u,0u));
    blu2usb_ux_shortcut_committed(&ux,0u,0u);
    assert(ux.shortcut_assignments[0] == 0u);
    assert(tap(&ux, BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
}

static void verify_mask_persistence_migration_and_no_modifier(void)
{
    blu2usb_profiles_t profile;
    blu2usb_profiles_init(&profile);
    assert(blu2usb_profiles_set_shortcut(&profile, 2u, 1u));
    assert(blu2usb_profiles_set_shortcut(&profile, 1u, 7u));
    assert(blu2usb_profiles_draft_set(
        &profile, BLU2USB_MOUSE_SOURCE_LEFT, BLU2USB_MOUSE_TARGET_MODIFIER));
    blu2usb_mouse_profile_config_t custom;
    assert(blu2usb_profiles_custom_candidate(&profile,&custom));
    blu2usb_profiles_activate(&profile,&custom);
    assert(blu2usb_profiles_is_shortcut_unavailable(&profile,2u));
    blu2usb_ux_model_t ux;
    enter_from_home(&ux);
    ux.shortcut_assignments[2] = profile.shortcuts[2];
    blu2usb_ux_set_modifier_source(&ux, BLU2USB_MOUSE_SOURCE_LEFT);
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_2;
    ux.selection=0u; /* MOD+LEFT masked */
    assert(blu2usb_ux_combination_unavailable(&ux,2u));
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind == BLU2USB_UX_COMMAND_NONE);
    assert(profile.shortcuts[2]==1u);
    assert(blu2usb_profiles_draft_set(
        &profile,BLU2USB_MOUSE_SOURCE_MIDDLE,BLU2USB_MOUSE_TARGET_MODIFIER));
    assert(blu2usb_profiles_custom_candidate(&profile,&custom));
    blu2usb_profiles_activate(&profile,&custom);
    blu2usb_ux_set_modifier_source(&ux,BLU2USB_MOUSE_SOURCE_MIDDLE);
    assert(!blu2usb_ux_combination_unavailable(&ux,2u));
    assert(blu2usb_profiles_is_shortcut_unavailable(&profile,1u));
    assert(profile.shortcuts[2]==1u && profile.shortcuts[1]==7u);
    uint8_t encoded[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(&profile,encoded));
    bool migrated=true;
    blu2usb_profiles_t recovered;
    assert(blu2usb_profiles_restore_any(&recovered,encoded,sizeof(encoded),&migrated));
    assert(!migrated && recovered.shortcuts[2]==1u && recovered.shortcuts[1]==7u);
    uint8_t corrupt[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    memcpy(corrupt,encoded,sizeof(corrupt));
    corrupt[BLU2USB_PROFILE_V3_SERIALIZED_SIZE-1u] ^= 0x55u;
    const blu2usb_profiles_t previous = recovered;
    assert(!blu2usb_profiles_restore_any(&recovered,corrupt,sizeof(corrupt),&migrated));
    assert(memcmp(&recovered,&previous,sizeof(recovered)) == 0); /* no overwrite */
    /* No configured Modifier => all assignments are inert for Mouse. */
    blu2usb_profiles_t passthrough;
    blu2usb_profiles_init(&passthrough);
    passthrough.shortcuts[3]=1u;
    blu2usb_mouse_profile_config_t cfg;
    assert(blu2usb_profiles_build_preset(
        BLU2USB_MOUSE_PROFILE_PASSTHROUGH,&passthrough,&cfg));
    blu2usb_modifier_shortcuts_t shortcuts;
    blu2usb_hid_aggregator_t agg;
    blu2usb_modifier_shortcuts_init(&shortcuts);
    blu2usb_hid_aggregator_init(&agg);
    blu2usb_modifier_shortcuts_configure(
        &shortcuts,&cfg,passthrough.shortcuts,&agg);
    assert(!shortcuts.enabled);
    blu2usb_canonical_mouse_event_t e={0};
    e.source=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE,1u);
    e.type=BLU2USB_MOUSE_EVENT_BUTTON;
    e.data.button.button=BLU2USB_MOUSE_BUTTON_RIGHT;
    e.data.button.pressed=true;
    assert(blu2usb_modifier_shortcuts_process(&shortcuts,&e));
    assert(shortcuts.queue_size==0u);
}

int main(void)
{
    verify_entire_picker_and_commit();
    verify_mask_persistence_migration_and_no_modifier();
    return 0;
}
