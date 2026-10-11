#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blu2usb/ux_model/ux_model.h"
#include "blu2usb/renderer/renderer.h"
#include "blu2usb/profiles/profiles.h"
#include "blu2usb/storage/storage.h"

static blu2usb_ux_command_t tap(blu2usb_ux_model_t *ux, blu2usb_control_t k)
{
    (void)blu2usb_ux_input(ux,k,true);
    return blu2usb_ux_input(ux,k,false);
}
static void connected(blu2usb_ux_model_t *ux)
{
    blu2usb_ux_set_mouse_connected(true);
    blu2usb_ux_init(ux);
    blu2usb_ux_set_saved_device_count(ux,1);
}
static void inspect(blu2usb_ux_model_t *ux,blu2usb_ui_frame_t *f)
{
    blu2usb_ui_project(ux,f);
    blu2usb_ui_enforce_applied_visual_contract(ux,f);
}
static void expect_row(const blu2usb_ui_frame_t *f,unsigned row,const char *expected)
{
    char actual[BLU2USB_RENDERER_TEXT_COLS+1];
    for(unsigned i=0;i<BLU2USB_RENDERER_TEXT_COLS;++i)
        actual[i]=f->cells[row][i].character;
    unsigned n=BLU2USB_RENDERER_TEXT_COLS;
    while(n>0 && actual[n-1]==' ')--n;
    actual[n]=0;
    assert(strcmp(expected,actual)==0);
}
static void expect_tone(const blu2usb_ui_frame_t *f,unsigned row,blu2usb_ui_tone_t color)
{
    unsigned used=0;
    for(unsigned i=0;i<BLU2USB_RENDERER_TEXT_COLS;++i)
        if(f->cells[row][i].character!=' ') {
            assert(f->cells[row][i].tone==color);
            ++used;
        }
    assert(used!=0);
}
static const char *const names[30]={
    "COPY","CUT","SELECT ALL","PASTE","UNDO","REDO",
    "ESCAPE","SPACE","ENTER\\RETURN","BACKSPACE","DELETE","RENAME",
    "FULL SCREEN","SCREENSHOT FULL","SCREENSHOT AREA","REFRESH","FIND","SAVE",
    "PLAY\\PAUSE","NEXT TRACK","PREVIOUS TRACK","VOLUME UP","VOLUME DOWN","MUTE",
    "HOME SCREEN\\PAGE","ZOOM IN","ZOOM OUT","RESET ZOOM","BRIGHTNESS UP","BRIGHTNESS DOWN"
};
static void assert_picker(blu2usb_ux_model_t *ux,unsigned page,unsigned selected)
{
    assert(ux->screen==(blu2usb_screen_id_t)(BLU2USB_SCREEN_SHORTCUTS_1+page));
    assert(ux->selection==selected);
    assert(blu2usb_ux_option_count(ux)==6);
    blu2usb_ui_frame_t frame;
    inspect(ux,&frame);
    char title[22];
    snprintf(title,sizeof(title),"SHORTCUTS %u-%u OF 30",page*6+1,page*6+6);
    expect_row(&frame,0,title);
    for(unsigned j=0;j<6;++j) {
        char text[22];
        snprintf(text,sizeof(text)," %s",names[page*6+j]);
        expect_row(&frame,j+1,text);
        expect_tone(&frame,j+1,j==selected?BLU2USB_UI_TONE_EMPHASIZED:BLU2USB_UI_TONE_ACTIONABLE);
    }
    expect_row(&frame,7,"");
    expect_row(&frame,8,"KEY A: APPLY AND BACK");
    assert(blu2usb_ux_selected_shortcut(ux)==page*6+selected+1);
}
static void enter_combo(blu2usb_ux_model_t *ux)
{
    tap(ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen==BLU2USB_SCREEN_MOUSE_OPTIONS);
    tap(ux,BLU2USB_CONTROL_JOY_DOWN);
    tap(ux,BLU2USB_CONTROL_JOY_DOWN);
    tap(ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(ux->screen==BLU2USB_SCREEN_COMBINATIONS_1);
}
static void test_pages_and_commit(void)
{
    blu2usb_ux_model_t ux;
    connected(&ux);
    enter_combo(&ux);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert_picker(&ux,0,0);
    for(unsigned page=0;page<5;++page) {
        assert_picker(&ux,page,0);
        tap(&ux,BLU2USB_CONTROL_JOY_RIGHT);
    }
    assert_picker(&ux,0,0); /* right wrap */
    tap(&ux,BLU2USB_CONTROL_JOY_LEFT);
    assert_picker(&ux,4,0); /* left wrap */
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert_picker(&ux,4,1);
    tap(&ux,BLU2USB_CONTROL_JOY_UP);
    assert_picker(&ux,4,0);
    tap(&ux,BLU2USB_CONTROL_JOY_UP);
    assert_picker(&ux,4,5); /* 0 -> 5 wrap */

    /* KEY A only emits intent: display stays in editor until flash succeeds. */
    blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_SET_SHORTCUT);
    assert(cmd.combination_id==0 && cmd.shortcut_id==30);
    assert_picker(&ux,4,5);
    assert(ux.shortcut_assignments[0]==0);
    /* Simulated verified store callback. */
    blu2usb_ux_shortcut_committed(&ux,cmd.combination_id,cmd.shortcut_id);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_1 && ux.selection==0);
    assert(ux.shortcut_assignments[0]==30);
    blu2usb_ui_frame_t frame;
    inspect(&ux,&frame);
    expect_row(&frame,2," BRIGHTNESS DOWN");
    expect_tone(&frame,1,BLU2USB_UI_TONE_EMPHASIZED);
    expect_tone(&frame,2,BLU2USB_UI_TONE_EMPHASIZED);
    expect_tone(&frame,3,BLU2USB_UI_TONE_ACTIONABLE);
    expect_tone(&frame,4,BLU2USB_UI_TONE_ACTIONABLE);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert_picker(&ux,4,5); /* prior selected restored */

    /* Cancelling always discards pending choice. */
    tap(&ux,BLU2USB_CONTROL_JOY_LEFT);
    assert_picker(&ux,3,5);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    assert_picker(&ux,3,0);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_1 && ux.selection==0);
    assert(ux.shortcut_assignments[0]==30);

    tap(&ux,BLU2USB_CONTROL_KEY_A);
    /* Reset must persist before mutating UI, only selected slot. */
    assert(ux.shortcut_assignments[0]==30);
}
static void test_reset_and_restored_combo(void)
{
    blu2usb_ux_model_t ux;
    connected(&ux);
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_3;
    ux.selection=1;
    assert(blu2usb_ux_selected_combination(&ux)==5);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert_picker(&ux,0,0);
    tap(&ux,BLU2USB_CONTROL_JOY_DOWN);
    blu2usb_ux_command_t cmd=tap(&ux,BLU2USB_CONTROL_KEY_A);
    assert(cmd.kind==BLU2USB_UX_COMMAND_SET_SHORTCUT);
    assert(cmd.shortcut_id==2 && cmd.combination_id==5);
    blu2usb_ux_shortcut_committed(&ux,5,2);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_3 && ux.selection==1);
    assert(ux.shortcut_assignments[5]==2);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_RESET_SHORTCUT);
    assert(ux.shortcut_assignments[5]==2); /* do not reflect until verified */
    blu2usb_ux_shortcut_committed(&ux,5,0);
    assert(ux.shortcut_assignments[5]==0);
    assert(tap(&ux,BLU2USB_CONTROL_KEY_A).kind==BLU2USB_UX_COMMAND_NONE);
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert_picker(&ux,0,0);
    assert(tap(&ux,BLU2USB_CONTROL_JOY_PRESS).kind==BLU2USB_UX_COMMAND_NONE);
    tap(&ux,BLU2USB_CONTROL_KEY_X);
    assert_picker(&ux,0,0);
    tap(&ux,BLU2USB_CONTROL_KEY_B);
    assert(ux.screen==BLU2USB_SCREEN_COMBINATIONS_3 && ux.selection==1);
}
static void test_v2_migration_and_v3_persistence(void)
{
    blu2usb_profiles_t v2;
    blu2usb_profiles_init(&v2);
    v2.active_kind=BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP;
    assert(blu2usb_profiles_draft_set(&v2,BLU2USB_MOUSE_SOURCE_LEFT,BLU2USB_MOUSE_TARGET_ESCAPE));
    /* legacy draft and custom ESCAPE are independently converted to identity */
    v2.custom_targets[BLU2USB_MOUSE_SOURCE_BACKWARD]=BLU2USB_MOUSE_TARGET_ESCAPE;
    uint8_t legacy[BLU2USB_PROFILE_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize(&v2,legacy));
    blu2usb_profiles_t candidate;
    bool migrated=false;
    assert(blu2usb_profiles_restore_any(&candidate,legacy,sizeof(legacy),&migrated));
    assert(migrated);
    assert(candidate.active_kind==BLU2USB_MOUSE_PROFILE_PASSTHROUGH);
    assert(candidate.custom_targets[BLU2USB_MOUSE_SOURCE_BACKWARD]==BLU2USB_MOUSE_TARGET_BACKWARD);
    assert(candidate.draft_targets[BLU2USB_MOUSE_SOURCE_LEFT]==BLU2USB_MOUSE_TARGET_LEFT);
    for(unsigned i=0;i<12;++i)assert(candidate.shortcuts[i]==0);
    assert(blu2usb_profiles_set_shortcut(&candidate,0,7));
    assert(blu2usb_profiles_set_shortcut(&candidate,11,30));
    assert(!blu2usb_profiles_set_shortcut(&candidate,12,1));
    assert(!blu2usb_profiles_set_shortcut(&candidate,0,31));

    uint8_t v3[BLU2USB_PROFILE_V3_SERIALIZED_SIZE];
    assert(blu2usb_profiles_serialize_v3(&candidate,v3));
    assert(v3[0]==3 && sizeof(v3)==26);
    uint8_t record[BLU2USB_STORAGE_RECORD_SIZE];
    uint8_t output[BLU2USB_STORAGE_MAX_PAYLOAD_SIZE];
    size_t size=0;
    assert(blu2usb_storage_record_encode(1,v3,sizeof(v3),record));
    assert(blu2usb_storage_record_decode(record,NULL,output,sizeof(output),&size));
    assert(size==26);
    blu2usb_profiles_t loaded;
    blu2usb_profiles_init(&loaded);
    migrated=true;
    assert(blu2usb_profiles_restore_any(&loaded,output,size,&migrated));
    assert(!migrated);
    assert(loaded.shortcuts[0]==7 && loaded.shortcuts[11]==30);
    assert(loaded.active_kind==BLU2USB_MOUSE_PROFILE_PASSTHROUGH);
    assert(loaded.custom_targets[BLU2USB_MOUSE_SOURCE_BACKWARD]==BLU2USB_MOUSE_TARGET_BACKWARD);
    loaded.draft_valid=false;
    assert(loaded.shortcuts[0]==7);
    /* No partial mutation on validation failures. */
    const blu2usb_profiles_t snapshot=loaded;
    v3[25]^=0x01u;
    assert(!blu2usb_profiles_restore_any(&loaded,v3,sizeof(v3),NULL));
    assert(memcmp(&loaded,&snapshot,sizeof(loaded))==0);
    v3[25]^=0x01u;
    v3[0]=4;
    assert(!blu2usb_profiles_restore_any(&loaded,v3,sizeof(v3),NULL));
    assert(memcmp(&loaded,&snapshot,sizeof(loaded))==0);
    /* Tamper with frame length and CRC; v2 and v3 are strictly sized. */
    assert(!blu2usb_profiles_restore_any(&loaded,legacy,sizeof(legacy)-1,NULL));
    record[16]^=0x33u;
    assert(!blu2usb_storage_record_decode(record,NULL,output,sizeof(output),&size));
    assert(memcmp(&loaded,&snapshot,sizeof(loaded))==0);
}
static void test_saved_shortcut_focus_and_lock(void)
{
    blu2usb_ux_model_t ux;
    connected(&ux);
    ux.shortcut_assignments[2]=8;
    ux.screen=BLU2USB_SCREEN_COMBINATIONS_2;
    ux.selection=0;
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert_picker(&ux,1,1); /* SPACE is ID 8 */
    tap(&ux,BLU2USB_CONTROL_JOY_LEFT);
    assert_picker(&ux,0,1);
    tap(&ux,BLU2USB_CONTROL_JOY_RIGHT);
    assert_picker(&ux,1,1); /* restore saved focus if on page */
    blu2usb_ui_frame_t f;
    inspect(&ux,&f);
    expect_tone(&f,2,BLU2USB_UI_TONE_EMPHASIZED);
    tap(&ux,BLU2USB_CONTROL_JOY_UP);
    inspect(&ux,&f);
    expect_tone(&f,2,BLU2USB_UI_TONE_CURRENT);
    tap(&ux,BLU2USB_CONTROL_KEY_Y);
    assert(blu2usb_interaction_is_locked(&ux.interaction));
    tap(&ux,BLU2USB_CONTROL_JOY_PRESS);
    assert(!blu2usb_interaction_is_locked(&ux.interaction));
    assert(ux.screen==BLU2USB_SCREEN_HOME);
    assert(ux.shortcut_assignments[2]==8);
}
int main(void)
{
    assert(BLU2USB_SCREEN_COUNT==45);
    for(unsigned i=1;i<=30;++i)assert(strcmp(blu2usb_ux_shortcut_name(i),names[i-1])==0);
    test_pages_and_commit();
    test_reset_and_restored_combo();
    test_v2_migration_and_v3_persistence();
    test_saved_shortcut_focus_and_lock();
    return 0;
}
