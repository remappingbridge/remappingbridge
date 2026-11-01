#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { TP_SLOTS = 5 };
typedef struct { bool down; uint16_t x, y; } tp_contact;
typedef struct { tp_contact finger[16]; bool click;
    /* Apple USB uses the same nine-byte records as Bluetooth. Keep pressure,
     * finger geometry and lifecycle flags intact for the native driver. */
    uint8_t records[TP_SLOTS * 9], record_count;
} tp_frame;
typedef struct { uint8_t flags, id, xlo, xhi, ylo, yhi; } tp_slot;
typedef struct {
    tp_slot slot[TP_SLOTS];
    uint8_t time_lo, time_hi, count, click;
} tp_report;
_Static_assert(sizeof(tp_report) == 34, "USB descriptor/report mismatch");
bool tp_decode(const uint8_t *data, size_t len, tp_frame *out);
void tp_encode(const tp_frame *frame, const tp_report *previous,
               uint16_t scan_time, tp_report *out);
size_t tp_apple_packet(const tp_frame *frame, uint16_t timestamp, uint8_t out[64]);
