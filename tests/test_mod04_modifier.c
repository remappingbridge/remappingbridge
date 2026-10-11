#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "blu2usb/profiles/profiles.h"
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t c)
{
    (void)blu2usb_ux_input(ux, c, true);
    return blu2usb_ux_input(ux, c, false);
}

static void row_text(const blu2usb_ui_frame_t *frame, unsigned row, char out[22])
{
    for (unsigned i = 0; i < 21; ++i) out[i] = frame->cells[row][i].character;
    unsigned size=21;
    while (size && out[size-1]==' ') --size;
    out[size]=0;
}

static void check_layout(void)
{
    static const blu2usb_screen_id_t pages[5] = {
        BLU2USB_SCREEN_LEFT_WILL_BECOME, BLU2USB_SCREEN_RIGHT_WILL_BECOME,
        BLU2USB_SCREEN_MIDDLE_WILL_BECOME, BLU2USB_SCREEN_FORWARD_WILL_BECOME,
        BLU2USB_SCREEN_BACKWARD_WILL_BECOME
    };
    static const char *const labels[6] = {
        " LEFT", " RIGHT", " MIDDLE", " FORWARD", " BACKWARD", " MODIFIER"
    };
    assert(BLU2USB_MOUSE_TARGET_ESCAPE == 5); /* historical v2 numeric ID */
    assert(BLU2USB_MOUSE_TARGET_MODIFIER == 6); /* new v3, never aliased */
    assert(BLU2USB_SCREEN_COUNT == 45);
    for (unsigned page=0;page<5;++page) {
        const blu2usb_screen_template_t *screen = blu2usb_ux_screen_template(pages[page]);
        assert(screen != NULL);
        for (unsigned row=1;row<=6;++row)
            assert(strcmp(screen->rows[row],labels[row-1])==0);
        assert(strcmp(screen->rows[8],"KEY A: APPLY AND BACK")==0);
        blu2usb_ux_model_t ux;
        blu2usb_ux_init(&ux);
        ux.screen = pages[page];
        assert(blu2usb_ux_option_count(&ux) == 6u);
        ux.selection=5;
        blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
        assert(cmd.kind==BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
        assert(cmd.target==BLU2USB_MOUSE_TARGET_MODIFIER);
        assert(ux.screen==pages[page]); /* write failure must remain here */
    }
    assert(strcmp(blu2usb_profiles_target_name(BLU2USB_MOUSE_TARGET_MODIFIER),"MODIFIER")==0);
}

static void test_exclusive_draft_and_restore(void)
{
    blu2usb_profiles_t profiles;
    blu2usb_profiles_init(&profiles);
    /* Operator can assign shortcuts before choosing a Modifier. */
    assert(blu2usb_profiles_set_shortcut(&profiles,2,1)); /* MOD+LEFT=COPY */
    assert(blu2usb_profiles_set_shortcut(&profiles,1,7)); /* MOD+MIDDLE=ESCAPE */
    assert(!blu2usb_profiles_is_shortcut_unavailable(&profiles,2));
    assert(blu2usb_profiles_draft_set(&profiles,BLU2USB_MOUSE_SOURCE_LEFT,
                                      BLU2USB_MOUSE_TARGET_MODIFIER));
    assert(!blu2usb_profiles_is_shortcut_unavailable(&profiles,2));
    blu2usb_mouse_profile_config_t applied;
    assert(blu2usb_profiles_custom_candidate(&profiles,&applied));
    blu2usb_profiles_activate(&profiles,&applied);
    assert(profiles.active_kind==BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP);
    assert(blu2usb_profiles_is_shortcut_unavailable(&profiles,2));
    assert(!blu2usb_profiles_is_shortcut_unavailable(&profiles,1));
    assert(profiles.shortcuts[2]==1);

    assert(blu2usb_profiles_draft_set(&profiles,BLU2USB_MOUSE_SOURCE_MIDDLE,
                                      BLU2USB_MOUSE_TARGET_MODIFIER));
    assert(profiles.draft_targets[BLU2USB_MOUSE_SOURCE_LEFT]==BLU2USB_MOUSE_TARGET_LEFT);
    assert(profiles.draft_targets[BLU2USB_MOUSE_SOURCE_MIDDLE]==BLU2USB_MOUSE_TARGET_MODIFIER);
    /* Previous applied Modifier is still LEFT until Custom APPLY succeeds. */
    assert(blu2usb_profiles_is_shortcut_unavailable(&profiles,2));
    assert(!blu2usb_profiles_is_shortcut_unavailable(&profiles,1));
    assert(blu2usb_profiles_custom_candidate(&profiles,&applied));
    blu2usb_profiles_activate(&profiles,&applied);
    assert(!blu2usb_profiles_is_shortcut_unavailable(&profiles,2));
    assert(blu2usb_profiles_is_shortcut_unavailable(&profiles,1));
    assert(profiles.shortcuts[2]==1 && profiles.shortcuts[1]==7);

    uint8_t encoded[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(&profiles,encoded));
    assert(encoded[0]==3u && encoded[3+BLU2USB_MOUSE_SOURCE_MIDDLE]==6u);
    blu2usb_profiles_t rebooted;
    bool migrated=true;
    assert(blu2usb_profiles_restore_any(&rebooted,encoded,sizeof(encoded),&migrated));
    assert(!migrated);
    assert(blu2usb_profiles_is_shortcut_unavailable(&rebooted,1));
    assert(rebooted.shortcuts[1]==7 && rebooted.shortcuts[2]==1);
    assert(rebooted.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT]==BLU2USB_MOUSE_TARGET_LEFT);

    /* Switching PASSTHROUGH does not remove masks from stored Custom settings. */
    blu2usb_mouse_profile_config_t pass;
    assert(blu2usb_profiles_build_preset(BLU2USB_MOUSE_PROFILE_PASSTHROUGH,&rebooted,&pass));
    blu2usb_profiles_activate(&rebooted,&pass);
    assert(blu2usb_profiles_is_shortcut_unavailable(&rebooted,1));
    assert(rebooted.shortcuts[1]==7);

    /* Invalid duplicate persisted Modifier must not be accepted, even with a
       repaired payload checksum; the original profile stays intact. */
    const blu2usb_profiles_t snapshot=rebooted;
    rebooted.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT]=BLU2USB_MOUSE_TARGET_MODIFIER;
    assert(!blu2usb_profiles_serialize_v3(&rebooted,encoded));
    rebooted=snapshot;
    assert(rebooted.shortcuts[2]==1);
}

static void test_masked_render_and_buttons(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    ux.modifier_source=BLU2USB_MOUSE_SOURCE_LEFT;
    ux.shortcut_assignments[2]=1;
    ux.shortcut_assignments[1]=7;
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_2;
    ux.selection=0; /* LEFT selected and masked */
    assert(blu2usb_ux_combination_unavailable(&ux,2));
    assert(!blu2usb_ux_combination_unavailable(&ux,0));
    assert(!blu2usb_ux_combination_unavailable(&ux,6));
    assert(!blu2usb_ux_combination_unavailable(&ux,10));
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_2);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.shortcut_assignments[2]==1);
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(&ux,&frame);
    blu2usb_ui_enforce_applied_visual_contract(&ux,&frame);
    char buffer[22];
    row_text(&frame,1,buffer); assert(strcmp(buffer," MOD + LEFT CLICK IS")==0);
    row_text(&frame,2,buffer); assert(strcmp(buffer," UNAVAILABLE")==0);
    assert(frame.cells[1][1].tone==BLU2USB_UI_TONE_EMPHASIZED);
    assert(frame.cells[2][1].tone==BLU2USB_UI_TONE_STATIC);
    assert(frame.cells[3][1].tone==BLU2USB_UI_TONE_ACTIONABLE);
    /* Previous value is recoverable after changing modifier; it never erased. */
    blu2usb_ux_set_modifier_source(&ux,BLU2USB_MOUSE_SOURCE_MIDDLE);
    blu2usb_ui_project(&ux,&frame);
    row_text(&frame,2,buffer); assert(strcmp(buffer," COPY")==0);
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen==BLU2USB_SCREEN_SHORTCUTS_1);
    assert(ux.selection==0);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_2 && ux.selection==0);
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_1;
    ux.selection=1; /* MIDDLE masked, ESCAPE retained */
    blu2usb_ui_project(&ux,&frame);
    row_text(&frame,4,buffer);assert(strcmp(buffer," UNAVAILABLE")==0);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
    assert(ux.shortcut_assignments[1]==7);
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_3;
    ux.selection=0;
    blu2usb_ui_project(&ux,&frame);
    row_text(&frame,2,buffer);assert(strcmp(buffer," NOT CUSTOMIZED")==0);

    /* No modifier source: all slots, including MIDDLE, available again. */
    blu2usb_ux_set_modifier_source(&ux,-1);
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_1;ux.selection=1;
    blu2usb_ui_project(&ux,&frame);
    row_text(&frame,4,buffer);assert(strcmp(buffer," ESCAPE")==0);
}
static void test_legacy_escape_migration(void)
{
    blu2usb_profiles_t legacy;
    blu2usb_profiles_init(&legacy);
    legacy.active_kind=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    legacy.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT]=BLU2USB_MOUSE_TARGET_ESCAPE;
    uint8_t old[BLU2USB_PROFILE_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize(&legacy,old));
    bool migrate=false;
    blu2usb_profiles_t next;
    assert(blu2usb_profiles_restore_any(&next,old,sizeof(old),&migrate));
    assert(migrate && next.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT]==BLU2USB_MOUSE_TARGET_LEFT);
    assert(blu2usb_profiles_modifier_source(next.custom_targets)==BLU2USB_MOUSE_SOURCE_COUNT);
    uint8_t v3[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(&next,v3));
    /* Previously written v3 (MOD-03) could also still hold Escape targets:
       use a valid checksum with intentionally legacy target 5. */
    v3[3]=BLU2USB_MOUSE_TARGET_ESCAPE;
    uint8_t check=0x5au;
    for (unsigned i=0;i<sizeof(v3)-1;++i) {
        check=(uint8_t)((check<<1)|(check>>7));
        check^=v3[i];
    }
    v3[sizeof(v3)-1]=check;
    migrate=false;
    assert(blu2usb_profiles_restore_any(&next,v3,sizeof(v3),&migrate));
    assert(migrate);
    assert(next.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT]==BLU2USB_MOUSE_TARGET_LEFT);
    assert(blu2usb_profiles_modifier_source(next.custom_targets)==BLU2USB_MOUSE_SOURCE_COUNT);
}
static void test_home_connected_to_modifier_and_mask(void)
{
    blu2usb_ux_model_t ux;
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(&ux);
    assert(ux.screen == BLU2USB_SCREEN_HOME);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 1u); /* Custom Remapping */
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_LEFT_WILL_BECOME);
    tap(&ux, BLU2USB_CONTROL_JOY_UP); /* wrap first to sixth */
    assert(ux.selection == 5u);
    blu2usb_ux_command_t cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind == BLU2USB_UX_COMMAND_CUSTOM_SET_TARGET);
    assert(cmd.source == BLU2USB_MOUSE_SOURCE_LEFT);
    assert(cmd.target == BLU2USB_MOUSE_TARGET_MODIFIER);
    assert(ux.screen == BLU2USB_SCREEN_LEFT_WILL_BECOME);
    /* A failed flash transaction does not navigate or commit the draft. */
    assert(ux.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_LEFT);
    blu2usb_profiles_t profiles;
    blu2usb_profiles_init(&profiles);
    assert(blu2usb_profiles_draft_set(&profiles, cmd.source, cmd.target));
    uint8_t stored[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(&profiles,stored));
    blu2usb_ux_custom_target_committed(&ux,cmd.source,profiles.draft_targets);
    assert(ux.screen == BLU2USB_SCREEN_EDIT_CUSTOM);
    assert(ux.selection == BLU2USB_MOUSE_SOURCE_LEFT);
    assert(ux.custom_targets[BLU2USB_MOUSE_SOURCE_LEFT] == BLU2USB_MOUSE_TARGET_MODIFIER);
    assert(ux.modifier_source == -1); /* Draft is not yet applied */
    cmd = tap(&ux, BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind == BLU2USB_UX_COMMAND_APPLY_CUSTOM);
    blu2usb_mouse_profile_config_t active;
    assert(blu2usb_profiles_custom_candidate(&profiles,&active));
    blu2usb_profiles_activate(&profiles,&active);
    assert(blu2usb_profiles_serialize_v3(&profiles,stored));
    const blu2usb_mouse_source_t modifier =
        blu2usb_profiles_modifier_source(profiles.custom_targets);
    assert(modifier == BLU2USB_MOUSE_SOURCE_LEFT);
    blu2usb_ux_set_modifier_source(&ux, (int)modifier);
    tap(&ux, BLU2USB_CONTROL_KEY_B);
    assert(ux.screen == BLU2USB_SCREEN_MOUSE_OPTIONS);
    tap(&ux, BLU2USB_CONTROL_JOY_DOWN);
    assert(ux.selection == 2u);
    tap(&ux, BLU2USB_CONTROL_JOY_PRESS);
    assert(ux.screen == BLU2USB_SCREEN_COMBINATIONS_1);
    tap(&ux, BLU2USB_CONTROL_JOY_RIGHT);
    assert(ux.screen == BLU2USB_SCREEN_COMBINATIONS_2);
    blu2usb_ui_frame_t frame;
    blu2usb_ui_project(&ux,&frame);
    char word[22];
    row_text(&frame,2,word);
    assert(strcmp(word," UNAVAILABLE") == 0);
    assert(tap(&ux, BLU2USB_CONTROL_JOY_PRESS).kind == BLU2USB_UX_COMMAND_NONE);
    assert(ux.screen == BLU2USB_SCREEN_COMBINATIONS_2);
}

static void test_each_source_masks_only_itself(void)
{
    const unsigned slot_by_source[BLU2USB_MOUSE_SOURCE_COUNT] = {
        2u, 3u, 1u, 4u, 5u
    };
    blu2usb_profiles_t p;
    blu2usb_profiles_init(&p);
    blu2usb_ux_model_t ux;
    blu2usb_ux_init(&ux);
    for (unsigned source=0; source<BLU2USB_MOUSE_SOURCE_COUNT; ++source) {
        blu2usb_profiles_init(&p);
        assert(blu2usb_profiles_draft_set(&p,(blu2usb_mouse_source_t)source,
                                           BLU2USB_MOUSE_TARGET_MODIFIER));
        blu2usb_mouse_profile_config_t config;
        assert(blu2usb_profiles_custom_candidate(&p,&config));
        blu2usb_profiles_activate(&p,&config);
        blu2usb_ux_set_modifier_source(&ux,(int)source);
        for (unsigned slot=0; slot<BLU2USB_MOD_COMBO_COUNT; ++slot) {
            const bool expected=slot==slot_by_source[source];
            assert(blu2usb_profiles_is_shortcut_unavailable(&p,slot)==expected);
            assert(blu2usb_ux_combination_unavailable(&ux,slot)==expected);
        }
    }
}

int main(void)
{
    check_layout();
    test_home_connected_to_modifier_and_mask();
    test_each_source_masks_only_itself();
    test_exclusive_draft_and_restore();
    test_masked_render_and_buttons();
    test_legacy_escape_migration();
    return 0;
}
