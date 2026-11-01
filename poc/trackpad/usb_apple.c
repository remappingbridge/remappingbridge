#include "usb.h"
#include "tusb.h"
#include "pico/time.h"
#include <string.h>

/* Device-capture values for the MT2 USB protocol. See README for provenance.
 * Interface 1 binds the Apple multitouch driver, interface 2 its actuator. */
enum { AUX, PAD, ACTUATOR };
static const uint8_t aux_desc[] = {
    0x06,0,0xff,0x09,1,0xa1,1,0x15,0,0x26,255,0,
    0x75,8,0x95,1,0x09,1,0x81,2,0xc0
};
static const uint8_t pad_desc[] = {
    0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x05, 0x09,
    0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x85, 0x02, 0x95, 0x03,
    0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01, 0x05, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x02,
    0x81, 0x06, 0x95, 0x04, 0x75, 0x08, 0x81, 0x01, 0xC0, 0xC0, 0x05, 0x0D,
    0x09, 0x05, 0xA1, 0x01, 0x06, 0x00, 0xFF, 0x09, 0x0C, 0x15, 0x00, 0x26,
    0xFF, 0x00, 0x75, 0x08, 0x95, 0x10, 0x85, 0x3F, 0x81, 0x22, 0xC0, 0x06,
    0x00, 0xFF, 0x09, 0x0C, 0xA1, 0x01, 0x06, 0x00, 0xFF, 0x09, 0x0C, 0x15,
    0x00, 0x26, 0xFF, 0x00, 0x85, 0x44, 0x75, 0x08, 0x96, 0x6B, 0x05, 0x81,
    0x00, 0xC0,
};
static const uint8_t actuator_desc[] = {
    0x06, 0x00, 0xFF, 0x09, 0x0D, 0xA1, 0x01, 0x06, 0x00, 0xFF, 0x09, 0x0D,
    0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x85, 0x3F, 0x96, 0x0F, 0x00,
    0x81, 0x02, 0x09, 0x0D, 0x85, 0x53, 0x96, 0x3F, 0x00, 0x91, 0x02, 0xC0,
};
static const uint8_t properties[] = {
    0x01, 0x02, 0x00, 0xd1, 0x81, 0x0f, 0x00, 0xd3, 0x01, 0x16, 0x1e, 0x05,
    0x15, 0x00, 0x14, 0x1e, 0x62, 0x05, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00,
    0xd0, 0x02, 0x01, 0x00, 0x14, 0x01, 0x00, 0x1e, 0x00, 0x02, 0x14, 0x02,
    0x01, 0x0e, 0x02, 0x00, 0x07, 0x00, 0xa1, 0x00, 0x00, 0x05, 0x00, 0xfa,
    0x01, 0x11, 0x00, 0xd9, 0xf0, 0x3c, 0x00, 0x00, 0x20, 0x2b, 0x00, 0x00,
    0x44, 0xe3, 0x52, 0xff, 0xbd, 0x1e, 0xe4, 0x26, 0x05, 0x00, 0x7f, 0x00,
    0x00, 0x00, 0x00,
};

static const tusb_desc_device_t device = {
    .bLength=sizeof(device), .bDescriptorType=TUSB_DESC_DEVICE, .bcdUSB=0x0200,
    .bMaxPacketSize0=64, .idVendor=0x05ac, .idProduct=0x0265, .bcdDevice=0x0871,
    .iManufacturer=1, .iProduct=2, .bNumConfigurations=1
};
static const uint8_t config[] = {
    TUD_CONFIG_DESCRIPTOR(1,3,0,TUD_CONFIG_DESC_LEN+2*TUD_HID_DESC_LEN+TUD_HID_INOUT_DESC_LEN,0,100),
    TUD_HID_DESCRIPTOR(AUX,0,HID_ITF_PROTOCOL_NONE,sizeof(aux_desc),0x81,64,1),
    TUD_HID_DESCRIPTOR(PAD,0,HID_ITF_PROTOCOL_NONE,sizeof(pad_desc),0x82,64,1),
    TUD_HID_INOUT_DESCRIPTOR(ACTUATOR,0,HID_ITF_PROTOCOL_NONE,sizeof(actuator_desc),0x03,0x83,64,1)
};
const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)&device; }
const uint8_t *tud_descriptor_configuration_cb(uint8_t i) { return i ? NULL : config; }
const uint8_t *tud_hid_descriptor_report_cb(uint8_t i) {
    static const uint8_t *const descriptors[] = {aux_desc,pad_desc,actuator_desc};
    return i < 3 ? descriptors[i] : NULL;
}
const uint16_t *tud_descriptor_string_cb(uint8_t i,uint16_t lang) {
    (void)lang;
    static uint16_t s[32];
    if (!i) { s[0]=0x0304; s[1]=0x0409; return s; }
    if (i>2) return NULL;
    const char *name=i==1 ? "RemappingBridge" : "Magic Trackpad";
    size_t len=strlen(name);
    s[0]=0x0300 | (2*len+2);
    for (size_t j=0;j<len;j++) s[j+1]=name[j];
    return s;
}

static uint8_t selector[4], settings[2], actuator[4][16], actuator_len[4];
static bool enabled, completed_contact;
static uint32_t last_tx;
void tud_mount_cb(void) {
    enabled=completed_contact=false;
    memcpy(selector,(uint8_t[]){0x99,0,2,0},4);
    memset(settings,0,sizeof(settings));
    memset(actuator_len,0,sizeof(actuator_len));
}
void tud_umount_cb(void) { enabled=completed_contact=false; }
static uint16_t copy_feature(uint8_t *buf,uint16_t len,const uint8_t *src,uint16_t size) {
    if (size>len) size=len;
    memcpy(buf,src,size);
    return size;
}
uint16_t tud_hid_get_report_cb(uint8_t i,uint8_t id,hid_report_type_t type,uint8_t *buf,uint16_t len) {
    if (type!=HID_REPORT_TYPE_FEATURE || !len) return 0;
    if (i==ACTUATOR && id>=0x21 && id<=0x24)
        return copy_feature(buf,len,actuator[id-0x21],actuator_len[id-0x21]);
    if (i!=PAD) return 0;
    switch (id) {
    case 0: return copy_feature(buf,len,(uint8_t[]){0,1},2);
    case 1: return copy_feature(buf,len,selector,sizeof(selector));
    case 2: buf[0]=enabled; return 1;
    case 0x99: return copy_feature(buf,len,settings,sizeof(settings));
    case 0xdb: return copy_feature(buf,len,properties,sizeof(properties));
    default: return 0;
    }
}
void tud_hid_set_report_cb(uint8_t i,uint8_t id,hid_report_type_t type,const uint8_t *buf,uint16_t len) {
    if (type!=HID_REPORT_TYPE_FEATURE || !len) return;
    if (i==ACTUATOR && id>=0x21 && id<=0x24) {
        if (len>16) len=16;
        memcpy(actuator[id-0x21],buf,len);
        actuator_len[id-0x21]=len;
    }
    if (i!=PAD) return;
    if (id==1 && len>=2 && buf[0]==1) {
        selector[0]=buf[1]; selector[1]=selector[3]=0;
        selector[2]=buf[1]==0xdb ? sizeof(properties) : buf[1]==0x99 ? sizeof(settings) : 0;
    } else if (id==2) {
        enabled=buf[0]!=0;
        completed_contact=false;
    } else if (id==0x99 && len==2) memcpy(settings,buf,2);
}
void tud_hid_report_complete_cb(uint8_t i,const uint8_t *data,uint16_t len) {
    if (enabled && i==PAD && len>=21 && data[0]==2) completed_contact=true;
}
void trackpad_usb_reset_delivery(void) { completed_contact=false; }
bool trackpad_usb_delivered(void) { return tud_mounted() && enabled && completed_contact; }
void trackpad_usb_init(void) { tud_init(0); }
void trackpad_usb_task(const tp_frame *latest,bool *pending) {
    tud_task();
    uint32_t now=time_us_32();
    if (!tud_mounted() || !enabled || !tud_hid_n_ready(PAD)) return;
    if (now-last_tx<4000 || (!*pending && !latest->record_count && !latest->click)) return;
    uint8_t packet[64];
    size_t size=tp_apple_packet(latest,(uint16_t)(time_us_64()/125),packet);
    /* Includes its report ID already. TinyUSB copies all bytes before returning. */
    if (tud_hid_n_report(PAD,0,packet,size)) {
        *pending=false;
        last_tx=now;
    }
}
