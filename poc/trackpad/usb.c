#include "tusb.h"
#include "usb.h"
#include "pico/time.h"
#include <string.h>

/* Each logical Finger collection occupies one tp_slot. Coordinates are
 * absolute, in a 160 x 114.9 mm plane (SI linear, hundredths of a cm). */
#define FINGER \
    0x05,13, 0x09,0x22, 0xa1,2, 0xa4, \
    0x09,0x42, 0x09,0x47, 0x15,0, 0x25,1, 0x75,1, 0x95,2, 0x81,2, \
    0x95,6, 0x81,3, 0x09,0x51, 0x25,15, 0x75,8, 0x95,1, 0x81,2, \
    0x05,1, 0x55,0x0e, 0x65,0x11, 0x75,16, \
    0x09,0x30, 0x26,0xbc,0x1d, 0x46,0x40,0x06, 0x81,2, \
    0x09,0x31, 0x26,0xc9,0x13, 0x46,0x7d,0x04, 0x81,2, 0xb4, 0xc0

static const uint8_t descriptor[] = {
    0x05,13, 0x09,5, 0xa1,1, 0x85,1,
    FINGER, FINGER, FINGER, FINGER, FINGER,
    0x09,0x56, 0x15,0, 0x27,255,255,0,0, 0x75,16, 0x95,1,
    0x55,0x0c, 0x66,1,16, 0x81,2, 0x55,0, 0x65,0,
    0x09,0x54, 0x25,5, 0x75,8, 0x81,2,
    0x05,9, 0x09,1, 0x25,1, 0x75,1, 0x81,2, 0x95,7, 0x81,3,
    0x05,13, 0x85,2, 0x09,0x55, 0x09,0x59, 0x25,15, 0x75,4, 0x95,2, 0xb1,2,
    0x06,0,0xff, 0x85,3, 0x09,0xc5, 0x26,255,0, 0x75,8, 0x96,0,1, 0xb1,2, 0xc0,
    0x05,13, 0x09,0x0e, 0xa1,1, 0x09,0x22, 0xa1,2,
    0x85,4, 0x09,0x52, 0x15,0, 0x25,3, 0x75,8, 0x95,1, 0xb1,2, 0xc0,
    0x09,0x22, 0xa1,0, 0x85,5, 0x09,0x57, 0x09,0x58,
    0x25,1, 0x75,1, 0x95,2, 0xb1,2, 0x95,6, 0xb1,3, 0xc0, 0xc0,
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(6))
};
static const tusb_desc_device_t device = {
    .bLength=sizeof(device), .bDescriptorType=TUSB_DESC_DEVICE, .bcdUSB=0x0200,
    .bMaxPacketSize0=64, .idVendor=0xcafe, .idProduct=0x4025, .bcdDevice=0x0100,
    .iProduct=1, .bNumConfigurations=1
};
static const uint8_t config[] = {
    TUD_CONFIG_DESCRIPTOR(1,1,0,TUD_CONFIG_DESC_LEN+TUD_HID_DESC_LEN,0,100),
    TUD_HID_DESCRIPTOR(0,0,HID_ITF_PROTOCOL_NONE,sizeof(descriptor),0x81,64,1)
};
const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)&device; }
const uint8_t *tud_descriptor_configuration_cb(uint8_t i) { return i ? NULL : config; }
const uint8_t *tud_hid_descriptor_report_cb(uint8_t i) { return i ? NULL : descriptor; }
const uint16_t *tud_descriptor_string_cb(uint8_t i, uint16_t lang) {
    (void)lang;
    static uint16_t s[32];
    const char *name = "RemappingBridge Trackpad POC";
    if (!i) { s[0]=0x0304; s[1]=0x0409; return s; }
    if (i != 1) return NULL;
    size_t n=strlen(name);
    s[0]=0x0300 | (2*n+2);
    for (size_t j=0; j<n; j++) s[j+1]=name[j];
    return s;
}

uint8_t tp_mode = 3, tp_switches = 3;
void tud_mount_cb(void) { tp_mode = tp_switches = 3; }
uint16_t tud_hid_get_report_cb(uint8_t i,uint8_t id,hid_report_type_t type,uint8_t *buf,uint16_t len) {
    if (i || type != HID_REPORT_TYPE_FEATURE || !len) return 0;
    switch (id) {
    case 2: buf[0]=5; return 1;
    case 3:
        if (len > 256) len = 256;
        memset(buf,0,len); return len; /* Uncertified development device. */
    case 4: buf[0]=tp_mode; return 1;
    case 5: buf[0]=tp_switches; return 1;
    default: return 0;
    }
}
void tud_hid_set_report_cb(uint8_t i,uint8_t id,hid_report_type_t type,const uint8_t *buf,uint16_t len) {
    if (i || type != HID_REPORT_TYPE_FEATURE || len != 1) return;
    if (id == 4 && (buf[0] == 0 || buf[0] == 3)) tp_mode=buf[0];
    if (id == 5) tp_switches=buf[0] & 3;
}

static tp_report delivered;
void trackpad_usb_init(void) { tud_init(0); }
void trackpad_usb_task(const tp_frame *latest, bool *pending) {
    tud_task();
    static uint8_t last_mode = 255, last_switches;
    if (!tud_mounted()) {
        delivered = (tp_report){0};
        *pending = true;
        last_mode = 255;
        return;
    }
    if (tp_mode != last_mode || tp_switches != last_switches) *pending = true;
    if (!*pending || !tud_hid_ready()) return;
    tp_frame f = *latest;
    if (!(tp_switches & 1)) memset(f.finger,0,sizeof(f.finger));
    if (!(tp_switches & 2)) f.click = false;
    if (tp_mode == 0) {
        /* Relative pointer compatibility when the host selects mouse mode. */
        static int prior_id = -1;
        static tp_contact prior;
        if (last_mode != 0) prior_id = -1;
        int id = 0;
        while (id < 16 && !f.finger[id].down) id++;
        int dx=0, dy=0;
        if (id < 16 && id == prior_id) {
            dx=((int)f.finger[id].x-prior.x)/8;
            dy=((int)f.finger[id].y-prior.y)/8;
        }
        if (dx > 127) dx=127;
        if (dx < -127) dx=-127;
        if (dy > 127) dy=127;
        if (dy < -127) dy=-127;
        if (!tud_hid_mouse_report(6,f.click,dx,dy,0,0)) return;
        prior_id=id;
        if (id < 16) prior=f.finger[id];
        delivered=(tp_report){0};
    } else {
        tp_report r;
        tp_encode(&f,&delivered,(uint16_t)(time_us_64()/100),&r);
        if (!tud_hid_report(1,&r,sizeof(r))) return;
        delivered=r;
    }
    last_switches=tp_switches;
    last_mode=tp_mode;
    *pending=false;
}

