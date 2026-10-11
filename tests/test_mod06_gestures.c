#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "blu2usb/modifier_shortcuts/modifier_shortcuts.h"
#include "blu2usb/movement/movement.h"

static blu2usb_mouse_profile_config_t custom(void) {
    blu2usb_mouse_profile_config_t p={0};
    p.kind=BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    p.targets[0]=BLU2USB_MOUSE_TARGET_MODIFIER;
    p.targets[1]=BLU2USB_MOUSE_TARGET_RIGHT;
    p.targets[2]=BLU2USB_MOUSE_TARGET_MIDDLE;
    p.targets[3]=BLU2USB_MOUSE_TARGET_FORWARD;
    p.targets[4]=BLU2USB_MOUSE_TARGET_BACKWARD;
    return p;
}
static blu2usb_canonical_mouse_event_t make_button(bool pressed) {
    blu2usb_canonical_mouse_event_t e={0};
    e.source=blu2usb_hid_source_make(BLU2USB_HID_SOURCE_MOUSE,1);
    e.type=BLU2USB_MOUSE_EVENT_BUTTON;
    e.data.button.button=BLU2USB_MOUSE_BUTTON_LEFT;
    e.data.button.pressed=pressed;return e;
}
static blu2usb_canonical_mouse_event_t make_move(int x,int y) {
    blu2usb_canonical_mouse_event_t e=make_button(false);
    e.type=BLU2USB_MOUSE_EVENT_MOVE;
    e.data.move.dx=(int16_t)x;e.data.move.dy=(int16_t)y;return e;
}
static blu2usb_canonical_mouse_event_t make_wheel(int v,int h) {
    blu2usb_canonical_mouse_event_t e=make_button(false);
    e.type=BLU2USB_MOUSE_EVENT_WHEEL;
    e.data.wheel.vertical=(int16_t)v;
    e.data.wheel.horizontal=(int16_t)h;return e;
}
static bool flow(blu2usb_modifier_shortcuts_t *state,
                 blu2usb_canonical_mouse_event_t event,
                 blu2usb_movement_settings_t settings,uint32_t time,
                 blu2usb_canonical_mouse_event_t *forwarded) {
    return blu2usb_modifier_shortcuts_process_event(state,&event,settings,time,forwarded);
}
static void start(blu2usb_modifier_shortcuts_t *state,
                  blu2usb_hid_aggregator_t *agg,const uint8_t ids[12]) {
    blu2usb_modifier_shortcuts_init(state);
    blu2usb_hid_aggregator_init(agg);
    const blu2usb_mouse_profile_config_t p=custom();
    blu2usb_modifier_shortcuts_configure(state,&p,ids,agg);
    assert(state->enabled);
}
static void press(blu2usb_modifier_shortcuts_t *s,blu2usb_movement_settings_t set) {
    blu2usb_canonical_mouse_event_t out;
    assert(!flow(s,make_button(true),set,0,&out));
}
static void release(blu2usb_modifier_shortcuts_t *s,blu2usb_movement_settings_t set) {
    blu2usb_canonical_mouse_event_t out;
    assert(!flow(s,make_button(false),set,1000,&out));
}
static uint8_t pending_id(const blu2usb_modifier_shortcuts_t *s,unsigned offset) {
    return s->pending[(s->queue_head+offset)%BLU2USB_MOD05_QUEUE_CAPACITY];
}
static void check_wheel(void) {
    blu2usb_modifier_shortcuts_t s;
    blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    blu2usb_movement_settings_t movement=blu2usb_movement_defaults();
    uint8_t ids[12]={0}; ids[0]=7;ids[6]=1;ids[7]=7;
    start(&s,&agg,ids);
    /* No modifier => all scroll, including horizontal pan, passes unchanged. */
    assert(flow(&s,make_wheel(2,3),movement,0,&out));
    assert(out.data.wheel.vertical==2 && out.data.wheel.horizontal==3);
    assert(s.queue_size==0);
    press(&s,movement);
    assert(flow(&s,make_wheel(2,3),movement,1,&out));
    assert(out.data.wheel.vertical==0 && out.data.wheel.horizontal==3);
    assert(s.queue_size==2 && pending_id(&s,0)==1 && pending_id(&s,1)==1);
    assert(!flow(&s,make_wheel(-3,0),movement,2,&out));
    assert(s.queue_size==5);
    for(unsigned i=2;i<5;++i) assert(pending_id(&s,i)==7);
    assert(flow(&s,make_wheel(0,4),movement,3,&out));
    assert(out.data.wheel.horizontal==4 && out.data.wheel.vertical==0);
    assert(s.queue_size==5);
    release(&s,movement);
    assert(s.queue_size==5); /* no extra modifier-released Escape */
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(s.queue_size==0);
    /* Unsupported semantic action does not impersonate keyboard; scroll passes. */
    ids[6]=19;ids[7]=0;
    start(&s,&agg,ids);press(&s,movement);
    assert(flow(&s,make_wheel(1,0),movement,1,&out));
    assert(out.data.wheel.vertical==1 && s.queue_size==0);
    release(&s,movement);
}
static void check_rotated_direction(unsigned rotation,int x,int y,
                                    unsigned expected_combo,uint8_t expected_id,
                                    unsigned h_gain,unsigned v_gain) {
    blu2usb_modifier_shortcuts_t s;blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    uint8_t ids[12]={0};ids[0]=7;
    ids[8]=1;ids[9]=7;ids[10]=1;ids[11]=7;
    start(&s,&agg,ids);
    blu2usb_movement_settings_t m={(uint8_t)h_gain,(uint8_t)v_gain,(uint8_t)rotation};
    assert(blu2usb_movement_valid(m));press(&s,m);
    /* First fragment captured without moving pointer or triggering. */
    assert(!flow(&s,make_move(x/3,y/3),m,10,&out));
    assert(s.queue_size==0);
    assert(!flow(&s,make_move(x,y),m,20,&out));
    assert(s.gesture_latched && s.queue_size==1);
    assert(pending_id(&s,0)==expected_id);
    int64_t rx=0,ry=0;blu2usb_movement_rotate_delta(m,x,y,&rx,&ry);
    unsigned actual=rx>0?9u:rx<0?8u:ry>0?11u:10u;
    assert(actual==expected_combo);
    /* Still held with repeated move => no extra action until genuine quiet. */
    for(int i=0;i<4;i++)
        assert(!flow(&s,make_move(x,y),m,(uint32_t)(30+i*10),&out));
    assert(s.queue_size==1);
    blu2usb_modifier_shortcuts_tick(&s,180);
    assert(s.gesture_latched); /* last movement at 60; still <160ms */
    blu2usb_modifier_shortcuts_tick(&s,240);
    assert(!s.gesture_latched);
    assert(!flow(&s,make_move(x,y),m,250,&out));
    assert(s.queue_size==2 && pending_id(&s,1)==expected_id);
    release(&s,m);
    assert(s.queue_size==2);
}
static void check_motion_capture_and_diagonal(void) {
    blu2usb_modifier_shortcuts_t s;blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    blu2usb_movement_settings_t m=blu2usb_movement_defaults();
    uint8_t ids[12]={0};ids[0]=7;ids[8]=1;ids[9]=7;ids[10]=1;ids[11]=7;
    start(&s,&agg,ids);
    /* Without Modifier cursor is NOT captured, even with MOVE assignments. */
    assert(flow(&s,make_move(4,6),m,0,&out));
    assert(out.data.move.dx==4 && out.data.move.dy==6);
    press(&s,m);
    assert(!flow(&s,make_move(1,-1),m,0,&out));
    assert(!flow(&s,make_move(2,0),m,1,&out));
    assert(s.queue_size==0 && !s.gesture_latched);
    /* Equal diagonal cannot fire BOTH axes or guess a direction. */
    assert(!flow(&s,make_move(30,30),m,2,&out));
    assert(s.queue_size==0 && !s.gesture_latched);
    blu2usb_modifier_shortcuts_tick(&s,200);
    assert(!flow(&s,make_move(-12,0),m,210,&out));
    assert(!flow(&s,make_move(12,0),m,211,&out)); /* reversal neutralizes */
    assert(s.queue_size==0);
    assert(!flow(&s,make_move(-25,0),m,220,&out));
    assert(s.queue_size==1 && pending_id(&s,0)==1);
    release(&s,m);
    assert(s.queue_size==1);
}
static void check_no_assignment_and_future_only(void) {
    blu2usb_modifier_shortcuts_t s;blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    blu2usb_movement_settings_t m=blu2usb_movement_defaults();
    uint8_t ids[12]={0};ids[0]=7;
    start(&s,&agg,ids);
    press(&s,m);
    assert(flow(&s,make_move(2,4),m,10,&out));
    assert(out.data.move.dx==2 && out.data.move.dy==4);
    release(&s,m);
    assert(s.queue_size==0); /* movement intentionally cancels tap threshold >= 8? 6 only => should still release */
}
static void check_future_action_capture(void) {
    blu2usb_modifier_shortcuts_t s;blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    blu2usb_movement_settings_t m=blu2usb_movement_defaults();
    uint8_t ids[12]={0}; ids[8]=19;ids[0]=7;
    start(&s,&agg,ids);press(&s,m);
    assert(!flow(&s,make_move(-40,0),m,10,&out));
    assert(s.gesture_latched && s.queue_size==0);
    release(&s,m);
    assert(s.queue_size==0);
    blu2usb_modifier_shortcuts_cancel(&s,&agg);
    assert(!s.gesture_latched && !s.modifier_down);
}
static void check_wheel_queue_overflow(void) {
    blu2usb_modifier_shortcuts_t s;blu2usb_hid_aggregator_t agg;
    blu2usb_canonical_mouse_event_t out;
    blu2usb_movement_settings_t m=blu2usb_movement_defaults();
    uint8_t ids[12]={0};ids[6]=1;
    start(&s,&agg,ids);press(&s,m);
    for(int i=0;i<3;++i)
        assert(!flow(&s,make_wheel(100,0),m,(uint32_t)i,&out));
    assert(s.queue_size==BLU2USB_MOD05_QUEUE_CAPACITY);
    release(&s,m);
    assert(s.queue_size==BLU2USB_MOD05_QUEUE_CAPACITY);
}
int main(void) {
    check_wheel();
    check_rotated_direction(0,-25,0,8,1,4,4);
    check_rotated_direction(0,25,0,9,7,8,4); /* H gain must not change threshold */
    check_rotated_direction(0,0,-25,10,1,4,8);
    check_rotated_direction(0,0,25,11,7,4,4);
    check_rotated_direction(1,25,0,10,1,8,4); /* rotation 90 => UP */
    check_rotated_direction(2,25,0,8,1,4,4);
    check_rotated_direction(3,25,0,11,7,4,8);
    check_motion_capture_and_diagonal();
    check_future_action_capture();
    check_wheel_queue_overflow();
    return 0;
}
