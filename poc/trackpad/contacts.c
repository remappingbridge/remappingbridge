#include "contacts.h"
#include <string.h>

static int signed13(unsigned value) {
    return (int)((value & 8191u) ^ 4096u) - 4096;
}

static uint16_t axis(int value, int maximum) {
    if (value < 0) return 0;
    if (value > maximum) return (uint16_t)maximum;
    return (uint16_t)value;
}

bool tp_decode(const uint8_t *data, size_t len, tp_frame *out) {
    if (len < 4 || data[0] != 0x31 || (len - 4) % 9 || len > 4 + 15 * 9)
        return false;
    tp_frame next = {0};
    next.click = (data[1] & 1) != 0;
    unsigned seen = 0;
    for (size_t i = 4; i < len; i += 9) {
        const uint8_t *p = data + i;
        unsigned id = p[8] & 15;
        if (seen & (1u << id)) return false;
        seen |= 1u << id;
        next.finger[id] = (tp_contact){
            (p[3] & 0xc0) == 0x80,
            axis(3678 + signed13(p[0] | (p[1] << 8)), 7612),
            axis(2478 - signed13((p[1] >> 5) | (p[2] << 3) | (p[3] << 11)), 5065)
        };
    }
    next.record_count = (len - 4) / 9;
    if (next.record_count > TP_SLOTS) next.record_count = TP_SLOTS;
    memcpy(next.records, data + 4, next.record_count * 9);
    *out = next;
    return true;
}

static tp_slot pressed(unsigned id, tp_contact p) {
    return (tp_slot){3, id, p.x, p.x >> 8, p.y, p.y >> 8};
}

void tp_encode(const tp_frame *frame, const tp_report *previous,
               uint16_t scan_time, tp_report *out) {
    tp_report report = {.time_lo = scan_time, .time_hi = scan_time >> 8,
                        .click = frame->click};
    unsigned retained = 0;
    for (unsigned i = 0; i < previous->count; i++) {
        tp_slot s = previous->slot[i];
        if (!(s.flags & 1)) continue;
        retained |= 1u << s.id;
        if (frame->finger[s.id].down) s = pressed(s.id, frame->finger[s.id]);
        else s.flags = 2; /* Explicit lift at the last delivered position. */
        report.slot[report.count++] = s;
    }
    for (unsigned id = 0; id < 16 && report.count < TP_SLOTS; id++) {
        if (frame->finger[id].down && !(retained & (1u << id)))
            report.slot[report.count++] = pressed(id, frame->finger[id]);
    }
    *out = report;
}

size_t tp_apple_packet(const tp_frame *frame, uint16_t timestamp, uint8_t out[64]) {
    memset(out, 0, 12);
    out[0] = 2;
    out[1] = frame->click;
    out[7] = frame->record_count != 0;
    out[8] = 0x31;
    out[9] = (uint8_t)timestamp;
    out[10] = (uint8_t)(timestamp >> 8);
    out[11] = 0x88;
    size_t bytes = frame->record_count * 9;
    memcpy(out + 12, frame->records, bytes);
    return 12 + bytes;
}
