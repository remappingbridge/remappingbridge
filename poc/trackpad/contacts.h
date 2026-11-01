#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { TP_SLOTS = 5 };
typedef struct { bool down; uint16_t x, y; } tp_contact;
typedef struct { tp_contact finger[16]; bool click; } tp_frame;
typedef struct { uint8_t flags, id, xlo, xhi, ylo, yhi; } tp_slot;
typedef struct {
    tp_slot slot[TP_SLOTS];
    uint8_t time_lo, time_hi, count, click;
} tp_report;
_Static_assert(sizeof(tp_report) == 34, "USB descriptor/report mismatch");
bool tp_decode(const uint8_t *data, size_t len, tp_frame *out);
void tp_encode(const tp_frame *frame, const tp_report *previous,
               uint16_t scan_time, tp_report *out);
