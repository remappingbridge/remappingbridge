#include <assert.h>
#include <limits.h>
#include "blu2usb/movement/movement.h"

static void drain(blu2usb_movement_pipeline_t *p, int64_t *x, int64_t *y)
{
    *x = *y = 0;
    for (int n = 0; n < 500000; ++n) {
        int8_t dx, dy;
        blu2usb_movement_pipeline_next(p, &dx, &dy);
        if (!dx && !dy) return;
        *x += dx; *y += dy;
        blu2usb_movement_pipeline_ack(p, dx, dy);
    }
    assert(0 && "excessive number of USB chunks");
}
int main(void)
{
    const uint8_t gains[] = {1,2,3,4,5,6,8,12};
    for (unsigned r=0;r<4;r++) for(unsigned h=0;h<8;h++) for(unsigned v=0;v<8;v++) {
        blu2usb_movement_settings_t s={gains[h],gains[v],(uint8_t)r};
        assert(blu2usb_movement_valid(s));
        blu2usb_movement_pipeline_t p={0};
        blu2usb_movement_pipeline_push(&p,s,400,-800);
        int64_t ox,oy; drain(&p,&ox,&oy);
        const int64_t rx=r==0?400:r==1?-800:r==2?-400:800;
        const int64_t ry=r==0?-800:r==1?-400:r==2?800:400;
        assert(ox==rx*gains[h]/4);
        assert(oy==ry*gains[v]/4);
    }
    blu2usb_movement_settings_t s={8,4,3};
    blu2usb_movement_pipeline_t p={0};
    blu2usb_movement_pipeline_push(&p,s,100,50);
    int64_t ox,oy; drain(&p,&ox,&oy);
    assert(ox==-100 && oy==100);
    blu2usb_movement_pipeline_clear(&p);
    s=(blu2usb_movement_settings_t){1,1,0};
    for(int i=0;i<4;i++) {
        blu2usb_movement_pipeline_push(&p,s,1,1);
        int8_t x,y; blu2usb_movement_pipeline_next(&p,&x,&y);
        assert(x == (i==3 ? 1 : 0));
        assert(y == (i==3 ? 1 : 0));
    }
    blu2usb_movement_pipeline_ack(&p,1,1);
    assert(p.pending_x_quarters==0 && p.pending_y_quarters==0);
    blu2usb_movement_pipeline_push(&p,(blu2usb_movement_settings_t){12,12,3},INT32_MIN,INT32_MAX);
    blu2usb_movement_pipeline_next(&p,(int8_t *)&ox,(int8_t *)&oy); /* no overflow in wide math */
    return 0;
}
