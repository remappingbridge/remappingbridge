#include "contacts.h"
#include <assert.h>
int main(void) {
    uint8_t raw[4 + 5 * 9] = {0x31, 1};
    for (unsigned i = 0; i < 5; i++) {
        raw[4 + i*9 + 3] = 0x80;
        raw[4 + i*9 + 8] = i + 9;
    }
    tp_frame f = {0};
    tp_report prev = {0}, out;
    assert(tp_decode(raw, sizeof raw, &f));
    tp_encode(&f, &prev, 0xabcd, &out);
    assert(out.count == 5 && out.click && out.time_hi == 0xab);
    assert(out.slot[4].id == 13 && out.slot[4].flags == 3);
    assert(f.finger[13].x == 3678 && f.finger[13].y == 2478);
    assert(!tp_decode(raw, sizeof raw - 1, &f));
    raw[12] = 10;
    assert(!tp_decode(raw, sizeof raw, &f));
    prev = out;
    raw[1] = 0;
    assert(tp_decode(raw, 4, &f));
    tp_encode(&f, &prev, 0, &out);
    assert(out.count == 5 && !out.click && out.slot[4].flags == 2);
    prev = out;
    tp_encode(&f, &prev, 1, &out);
    assert(out.count == 0);
}
