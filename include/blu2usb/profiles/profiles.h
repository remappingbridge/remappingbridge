#ifndef BLU2USB_PROFILES_PROFILES_H
#define BLU2USB_PROFILES_PROFILES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blu2usb/domain/profile.h"

#define BLU2USB_PROFILE_SERIALIZED_SIZE 14u /* Legacy v2 payload, read-only compatibility */
#define BLU2USB_PROFILE_V3_SERIALIZED_SIZE 26u
#define BLU2USB_SHORTCUT_COUNT 30u
#define BLU2USB_SHORTCUT_SLOT_COUNT 12u

typedef struct {
    blu2usb_mouse_profile_kind_t active_kind;
    blu2usb_mouse_target_t custom_targets[BLU2USB_MOUSE_SOURCE_COUNT];
    blu2usb_mouse_target_t draft_targets[BLU2USB_MOUSE_SOURCE_COUNT];
    bool draft_valid;
    uint8_t shortcuts[BLU2USB_SHORTCUT_SLOT_COUNT]; /* 0=NOT CUSTOMIZED, 1..30 */
} blu2usb_profiles_t;

void blu2usb_profiles_init(blu2usb_profiles_t *profiles);
void blu2usb_profiles_configure_active(const blu2usb_profiles_t *profiles,
                                       blu2usb_mouse_profile_config_t *config);
bool blu2usb_profiles_build_preset(blu2usb_mouse_profile_kind_t kind,
                                    const blu2usb_profiles_t *profiles,
                                    blu2usb_mouse_profile_config_t *config);
void blu2usb_profiles_activate(blu2usb_profiles_t *profiles,
                               const blu2usb_mouse_profile_config_t *config);
bool blu2usb_profiles_draft_set(blu2usb_profiles_t *profiles,
                                 blu2usb_mouse_source_t source,
                                 blu2usb_mouse_target_t target);
bool blu2usb_profiles_custom_candidate(const blu2usb_profiles_t *profiles,
                                        blu2usb_mouse_profile_config_t *config);
blu2usb_mouse_target_t blu2usb_profiles_target_for_source(
    const blu2usb_mouse_profile_config_t *config,
    blu2usb_mouse_source_t source);
bool blu2usb_profiles_requires_forward_held_fix(
    const blu2usb_mouse_profile_config_t *config);
bool blu2usb_profiles_serialize(const blu2usb_profiles_t *profiles,
                                uint8_t out[BLU2USB_PROFILE_SERIALIZED_SIZE]);
bool blu2usb_profiles_restore(blu2usb_profiles_t *profiles,
                              const uint8_t data[BLU2USB_PROFILE_SERIALIZED_SIZE]);
/* v3: version, active, draft_valid, 5 applied targets, 5 draft targets,
 * 12 shortcuts, integrity checksum. V2 is never rewritten as v2 at runtime. */
bool blu2usb_profiles_serialize_v3(const blu2usb_profiles_t *profiles,
    uint8_t out[BLU2USB_PROFILE_V3_SERIALIZED_SIZE]);
bool blu2usb_profiles_restore_any(blu2usb_profiles_t *profiles,
    const uint8_t *data, size_t length, bool *migrated_v2);
bool blu2usb_profiles_set_shortcut(blu2usb_profiles_t *profiles,
    unsigned combo_id, uint8_t shortcut_id);
const char *blu2usb_profiles_target_name(blu2usb_mouse_target_t target);

#endif
