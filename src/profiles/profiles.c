#include "blu2usb/profiles/profiles.h"

#include <string.h>

#define PROFILE_SCHEMA_VERSION 2u
#define PROFILE_ACTIVE_OFFSET 1u
#define PROFILE_DRAFT_VALID_OFFSET 2u
#define PROFILE_CUSTOM_OFFSET 3u
#define PROFILE_DRAFT_OFFSET (PROFILE_CUSTOM_OFFSET + BLU2USB_MOUSE_SOURCE_COUNT)

static bool source_valid(blu2usb_mouse_source_t source)
{
    return (unsigned)source < BLU2USB_MOUSE_SOURCE_COUNT;
}

static bool target_valid(blu2usb_mouse_target_t target)
{
    return (unsigned)target < BLU2USB_MOUSE_TARGET_COUNT;
}

/* Reserved v2 ESCAPE target (5) is legal only for v2 codec; the
 * v3 mapping uses explicit MODIFIER (6) and never aliases ESCAPE. */
static bool target_v3_valid(blu2usb_mouse_target_t target)
{
    return target_valid(target) && target != BLU2USB_MOUSE_TARGET_ESCAPE;
}

blu2usb_mouse_source_t blu2usb_profiles_modifier_source(
    const blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT])
{
    if (targets == NULL) return (blu2usb_mouse_source_t)BLU2USB_MOUSE_SOURCE_COUNT;
    for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i)
        if (targets[i] == BLU2USB_MOUSE_TARGET_MODIFIER)
            return (blu2usb_mouse_source_t)i;
    return (blu2usb_mouse_source_t)BLU2USB_MOUSE_SOURCE_COUNT;
}

bool blu2usb_profiles_is_shortcut_unavailable(const blu2usb_profiles_t *profiles,
                                               unsigned combination_id)
{
    if (profiles == NULL || combination_id >= BLU2USB_SHORTCUT_SLOT_COUNT)
        return false;
    const blu2usb_mouse_source_t src =
        blu2usb_profiles_modifier_source(profiles->custom_targets);
    /* The 12 stable combo ordinals are: released, middle, left, right,
     * forward, backward, wheel up/down, move L/R/U/D. */
    switch (combination_id) {
    case 1u: return src == BLU2USB_MOUSE_SOURCE_MIDDLE;
    case 2u: return src == BLU2USB_MOUSE_SOURCE_LEFT;
    case 3u: return src == BLU2USB_MOUSE_SOURCE_RIGHT;
    case 4u: return src == BLU2USB_MOUSE_SOURCE_FORWARD;
    case 5u: return src == BLU2USB_MOUSE_SOURCE_BACKWARD;
    default: return false;
    }
}

static bool unique_modifier(const blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT])
{
    unsigned modifier_count = 0u;
    for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
        if (!target_v3_valid(targets[i])) return false;
        if (targets[i] == BLU2USB_MOUSE_TARGET_MODIFIER)
            ++modifier_count;
    }
    return modifier_count <= 1u;
}

static blu2usb_mouse_target_t identity_target(blu2usb_mouse_source_t source)
{
    switch (source) {
    case BLU2USB_MOUSE_SOURCE_LEFT: return BLU2USB_MOUSE_TARGET_LEFT;
    case BLU2USB_MOUSE_SOURCE_RIGHT: return BLU2USB_MOUSE_TARGET_RIGHT;
    case BLU2USB_MOUSE_SOURCE_MIDDLE: return BLU2USB_MOUSE_TARGET_MIDDLE;
    case BLU2USB_MOUSE_SOURCE_FORWARD: return BLU2USB_MOUSE_TARGET_FORWARD;
    case BLU2USB_MOUSE_SOURCE_BACKWARD: return BLU2USB_MOUSE_TARGET_BACKWARD;
    default: return BLU2USB_MOUSE_TARGET_LEFT;
    }
}

static void identity_targets(blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT])
{
    for (unsigned source = 0u; source < BLU2USB_MOUSE_SOURCE_COUNT; ++source)
        targets[source] = identity_target((blu2usb_mouse_source_t)source);
}

void blu2usb_profiles_init(blu2usb_profiles_t *profiles)
{
    if (profiles == NULL) return;
    memset(profiles, 0, sizeof(*profiles));
    profiles->active_kind = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
    identity_targets(profiles->custom_targets);
    identity_targets(profiles->draft_targets);
}

static void fill_targets(blu2usb_mouse_profile_kind_t kind,
                         const blu2usb_profiles_t *profiles,
                         blu2usb_mouse_target_t targets[BLU2USB_MOUSE_SOURCE_COUNT])
{
    identity_targets(targets);
    switch (kind) {
    case BLU2USB_MOUSE_PROFILE_DEFAULT_REMAP:
        targets[BLU2USB_MOUSE_SOURCE_LEFT] = BLU2USB_MOUSE_TARGET_FORWARD;
        targets[BLU2USB_MOUSE_SOURCE_RIGHT] = BLU2USB_MOUSE_TARGET_BACKWARD;
        targets[BLU2USB_MOUSE_SOURCE_FORWARD] = BLU2USB_MOUSE_TARGET_LEFT;
        targets[BLU2USB_MOUSE_SOURCE_BACKWARD] = BLU2USB_MOUSE_TARGET_RIGHT;
        break;
    case BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP:
        targets[BLU2USB_MOUSE_SOURCE_LEFT] = BLU2USB_MOUSE_TARGET_ESCAPE;
        targets[BLU2USB_MOUSE_SOURCE_RIGHT] = BLU2USB_MOUSE_TARGET_BACKWARD;
        targets[BLU2USB_MOUSE_SOURCE_MIDDLE] = BLU2USB_MOUSE_TARGET_FORWARD;
        targets[BLU2USB_MOUSE_SOURCE_FORWARD] = BLU2USB_MOUSE_TARGET_LEFT;
        targets[BLU2USB_MOUSE_SOURCE_BACKWARD] = BLU2USB_MOUSE_TARGET_RIGHT;
        break;
    case BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP:
        if (profiles != NULL)
            memcpy(targets, profiles->custom_targets, sizeof(profiles->custom_targets));
        break;
    case BLU2USB_MOUSE_PROFILE_PASSTHROUGH:
    default:
        break;
    }
}

void blu2usb_profiles_configure_active(const blu2usb_profiles_t *profiles,
                                       blu2usb_mouse_profile_config_t *config)
{
    if (profiles == NULL || config == NULL) return;
    config->kind = profiles->active_kind;
    fill_targets(config->kind, profiles, config->targets);
}

bool blu2usb_profiles_build_preset(blu2usb_mouse_profile_kind_t kind,
                                    const blu2usb_profiles_t *profiles,
                                    blu2usb_mouse_profile_config_t *config)
{
    if (config == NULL || kind < BLU2USB_MOUSE_PROFILE_PASSTHROUGH ||
        kind > BLU2USB_MOUSE_PROFILE_ESCAPE_REMAP) return false;
    config->kind = kind;
    fill_targets(kind, profiles, config->targets);
    return true;
}

void blu2usb_profiles_activate(blu2usb_profiles_t *profiles,
                               const blu2usb_mouse_profile_config_t *config)
{
    if (profiles == NULL || config == NULL ||
        config->kind < BLU2USB_MOUSE_PROFILE_PASSTHROUGH ||
        config->kind > BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP) return;
    if (config->kind == BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP) {
        /* Legacy v2 tests still activate historical ESCAPE targets before
         * invoking the legacy serializer. The v3 writer rejects ESCAPE. */
        unsigned count_modifier = 0u;
        for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
            if (!target_valid(config->targets[i])) return;
            if (config->targets[i] == BLU2USB_MOUSE_TARGET_MODIFIER)
                ++count_modifier;
        }
        if (count_modifier > 1u) return;
        memcpy(profiles->custom_targets, config->targets, sizeof(profiles->custom_targets));
        memcpy(profiles->draft_targets, config->targets, sizeof(profiles->draft_targets));
        profiles->draft_valid = false;
    }
    profiles->active_kind = config->kind;
}

bool blu2usb_profiles_draft_set(blu2usb_profiles_t *profiles,
                                 blu2usb_mouse_source_t source,
                                 blu2usb_mouse_target_t target)
{
    if (profiles == NULL || !source_valid(source) || !target_valid(target)) return false;
    if (!profiles->draft_valid) {
        memcpy(profiles->draft_targets, profiles->custom_targets, sizeof(profiles->draft_targets));
        profiles->draft_valid = true;
    }
    profiles->draft_targets[source] = target;
    if (target == BLU2USB_MOUSE_TARGET_MODIFIER) {
        /* Explicit exclusive choice: the previous modifier source returns
         * to its own physical identity without deleting shortcut slots. */
        for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
            if (i != (unsigned)source &&
                profiles->draft_targets[i] == BLU2USB_MOUSE_TARGET_MODIFIER)
                profiles->draft_targets[i] = identity_target((blu2usb_mouse_source_t)i);
        }
    }
    return true;
}

bool blu2usb_profiles_custom_candidate(const blu2usb_profiles_t *profiles,
                                        blu2usb_mouse_profile_config_t *config)
{
    if (profiles == NULL || config == NULL) return false;
    config->kind = BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP;
    memcpy(config->targets,
           profiles->draft_valid ? profiles->draft_targets : profiles->custom_targets,
           sizeof(config->targets));
    return true;
}

blu2usb_mouse_target_t blu2usb_profiles_target_for_source(
    const blu2usb_mouse_profile_config_t *config,
    blu2usb_mouse_source_t source)
{
    if (config == NULL || !source_valid(source)) return BLU2USB_MOUSE_TARGET_LEFT;
    return config->targets[source];
}

bool blu2usb_profiles_requires_forward_held_fix(const blu2usb_mouse_profile_config_t *config)
{
    return config != NULL &&
        blu2usb_profiles_target_for_source(config, BLU2USB_MOUSE_SOURCE_FORWARD) !=
            BLU2USB_MOUSE_TARGET_FORWARD;
}

static uint8_t checksum(const uint8_t *data)
{
    uint8_t value = 0x5au;
    for (size_t i = 0u; i < BLU2USB_PROFILE_SERIALIZED_SIZE - 1u; ++i) {
        value = (uint8_t)((value << 1u) | (value >> 7u));
        value ^= data[i];
    }
    return value;
}

bool blu2usb_profiles_serialize(const blu2usb_profiles_t *profiles,
                                uint8_t out[BLU2USB_PROFILE_SERIALIZED_SIZE])
{
    if (profiles == NULL || out == NULL ||
        profiles->active_kind > BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP) return false;

    memset(out, 0, BLU2USB_PROFILE_SERIALIZED_SIZE);
    out[0] = PROFILE_SCHEMA_VERSION;
    out[PROFILE_ACTIVE_OFFSET] = (uint8_t)profiles->active_kind;
    out[PROFILE_DRAFT_VALID_OFFSET] = profiles->draft_valid ? 1u : 0u;

    for (size_t i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
        const blu2usb_mouse_target_t custom = profiles->custom_targets[i];
        const blu2usb_mouse_target_t draft =
            profiles->draft_valid ? profiles->draft_targets[i] : custom;
        if (!target_valid(custom) || !target_valid(draft)) return false;
        out[PROFILE_CUSTOM_OFFSET + i] = (uint8_t)custom;
        out[PROFILE_DRAFT_OFFSET + i] = (uint8_t)draft;
    }

    out[BLU2USB_PROFILE_SERIALIZED_SIZE - 1u] = checksum(out);
    return true;
}

bool blu2usb_profiles_restore(blu2usb_profiles_t *profiles,
                              const uint8_t data[BLU2USB_PROFILE_SERIALIZED_SIZE])
{
    if (profiles == NULL || data == NULL || data[0] != PROFILE_SCHEMA_VERSION ||
        data[PROFILE_ACTIVE_OFFSET] > BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP ||
        data[PROFILE_DRAFT_VALID_OFFSET] > 1u ||
        data[BLU2USB_PROFILE_SERIALIZED_SIZE - 1u] != checksum(data)) return false;

    blu2usb_profiles_t candidate;
    blu2usb_profiles_init(&candidate);
    candidate.active_kind =
        (blu2usb_mouse_profile_kind_t)data[PROFILE_ACTIVE_OFFSET];
    candidate.draft_valid = data[PROFILE_DRAFT_VALID_OFFSET] != 0u;

    for (size_t i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
        const blu2usb_mouse_target_t custom =
            (blu2usb_mouse_target_t)data[PROFILE_CUSTOM_OFFSET + i];
        const blu2usb_mouse_target_t draft =
            (blu2usb_mouse_target_t)data[PROFILE_DRAFT_OFFSET + i];
        if ((unsigned)custom > (unsigned)BLU2USB_MOUSE_TARGET_ESCAPE ||
            (unsigned)draft > (unsigned)BLU2USB_MOUSE_TARGET_ESCAPE)
            return false;
        candidate.custom_targets[i] = custom;
        candidate.draft_targets[i] = candidate.draft_valid ? draft : custom;
    }

    *profiles = candidate;
    return true;
}


/* A separate v3 codec preserves the tested legacy v2 decoder.  Neither
 * the Bluetooth credential sectors nor the per-device Movement TLV live
 * in this global product profile record. */
#define PROFILE_V3_SCHEMA_VERSION 3u
#define PROFILE_V3_SHORTCUT_OFFSET 13u

static uint8_t checksum_v3(const uint8_t *data)
{
    uint8_t value = 0x5au;
    for (size_t i = 0u; i + 1u < BLU2USB_PROFILE_V3_SERIALIZED_SIZE; ++i) {
        value = (uint8_t)((value << 1u) | (value >> 7u));
        value ^= data[i];
    }
    return value;
}

bool blu2usb_profiles_set_shortcut(blu2usb_profiles_t *profiles,
                                   unsigned combo_id, uint8_t shortcut_id)
{
    if (profiles == NULL || combo_id >= BLU2USB_SHORTCUT_SLOT_COUNT ||
        shortcut_id > BLU2USB_SHORTCUT_COUNT) return false;
    profiles->shortcuts[combo_id] = shortcut_id;
    return true;
}

bool blu2usb_profiles_serialize_v3(const blu2usb_profiles_t *profiles,
    uint8_t out[BLU2USB_PROFILE_V3_SERIALIZED_SIZE])
{
    if (profiles == NULL || out == NULL ||
        (profiles->active_kind != BLU2USB_MOUSE_PROFILE_PASSTHROUGH &&
         profiles->active_kind != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP))
        return false;
    uint8_t candidate[BLU2USB_PROFILE_V3_SERIALIZED_SIZE] = {0};
    candidate[0] = PROFILE_V3_SCHEMA_VERSION;
    candidate[PROFILE_ACTIVE_OFFSET] = (uint8_t)profiles->active_kind;
    candidate[PROFILE_DRAFT_VALID_OFFSET] = profiles->draft_valid ? 1u : 0u;
    if (!unique_modifier(profiles->custom_targets) ||
        (profiles->draft_valid && !unique_modifier(profiles->draft_targets)))
        return false;
    for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
        const blu2usb_mouse_target_t custom = profiles->custom_targets[i];
        const blu2usb_mouse_target_t draft = profiles->draft_valid
            ? profiles->draft_targets[i] : custom;
        candidate[PROFILE_CUSTOM_OFFSET+i] = (uint8_t)custom;
        candidate[PROFILE_DRAFT_OFFSET+i] = (uint8_t)draft;
    }
    for (unsigned i = 0u; i < BLU2USB_SHORTCUT_SLOT_COUNT; ++i) {
        if (profiles->shortcuts[i] > BLU2USB_SHORTCUT_COUNT) return false;
        candidate[PROFILE_V3_SHORTCUT_OFFSET+i] = profiles->shortcuts[i];
    }
    candidate[BLU2USB_PROFILE_V3_SERIALIZED_SIZE-1u] = checksum_v3(candidate);
    memcpy(out, candidate, sizeof(candidate));
    return true;
}

bool blu2usb_profiles_restore_any(blu2usb_profiles_t *profiles,
    const uint8_t *data, size_t length, bool *migrated_v2)
{
    if (profiles == NULL || data == NULL) return false;
    if (migrated_v2 != NULL) *migrated_v2 = false;
    blu2usb_profiles_t candidate;
    blu2usb_profiles_init(&candidate);
    if (length == BLU2USB_PROFILE_SERIALIZED_SIZE &&
        data[0] == PROFILE_SCHEMA_VERSION) {
        if (!blu2usb_profiles_restore(&candidate, data)) return false;
        /* Legacy presets are retired. Legacy ESCAPE Custom/draft targets
         * become their physical identity rather than a future MODIFIER. */
        if (candidate.active_kind != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP)
            candidate.active_kind = BLU2USB_MOUSE_PROFILE_PASSTHROUGH;
        for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
            const blu2usb_mouse_target_t identity =
                identity_target((blu2usb_mouse_source_t)i);
            if (candidate.custom_targets[i] == BLU2USB_MOUSE_TARGET_ESCAPE)
                candidate.custom_targets[i] = identity;
            if (candidate.draft_targets[i] == BLU2USB_MOUSE_TARGET_ESCAPE)
                candidate.draft_targets[i] = identity;
        }
        if (migrated_v2 != NULL) *migrated_v2 = true;
    } else if (length == BLU2USB_PROFILE_V3_SERIALIZED_SIZE &&
               data[0] == PROFILE_V3_SCHEMA_VERSION) {
        if (data[PROFILE_ACTIVE_OFFSET] != BLU2USB_MOUSE_PROFILE_PASSTHROUGH &&
            data[PROFILE_ACTIVE_OFFSET] != BLU2USB_MOUSE_PROFILE_CUSTOM_REMAP)
            return false;
        if (data[PROFILE_DRAFT_VALID_OFFSET] > 1u ||
            data[length-1u] != checksum_v3(data))
            return false;
        candidate.active_kind =
            (blu2usb_mouse_profile_kind_t)data[PROFILE_ACTIVE_OFFSET];
        candidate.draft_valid = data[PROFILE_DRAFT_VALID_OFFSET] != 0u;
        /* MOD-03's v3 could still contain legacy ESCAPE=5. Safely
         * canonicalize it to source identity, never to MODIFIER=6. */
        bool rewrite_legacy_escape = false;
        for (unsigned i = 0u; i < BLU2USB_MOUSE_SOURCE_COUNT; ++i) {
            blu2usb_mouse_target_t custom =
                (blu2usb_mouse_target_t)data[PROFILE_CUSTOM_OFFSET+i];
            blu2usb_mouse_target_t draft =
                (blu2usb_mouse_target_t)data[PROFILE_DRAFT_OFFSET+i];
            if (!target_valid(custom) || !target_valid(draft)) return false;
            if (custom == BLU2USB_MOUSE_TARGET_ESCAPE) {
                custom = identity_target((blu2usb_mouse_source_t)i);
                rewrite_legacy_escape = true;
            }
            if (draft == BLU2USB_MOUSE_TARGET_ESCAPE) {
                draft = identity_target((blu2usb_mouse_source_t)i);
                rewrite_legacy_escape = true;
            }
            candidate.custom_targets[i] = custom;
            candidate.draft_targets[i] = candidate.draft_valid ? draft : custom;
        }
        if (!unique_modifier(candidate.custom_targets) ||
            (candidate.draft_valid && !unique_modifier(candidate.draft_targets)))
            return false;
        if (rewrite_legacy_escape && migrated_v2 != NULL)
            *migrated_v2 = true; /* Existing boot migration writer is reusable. */
        for (unsigned i = 0u; i < BLU2USB_SHORTCUT_SLOT_COUNT; ++i) {
            const uint8_t id = data[PROFILE_V3_SHORTCUT_OFFSET+i];
            if (id > BLU2USB_SHORTCUT_COUNT) return false;
            candidate.shortcuts[i] = id;
        }
    } else return false;
    *profiles = candidate;
    return true;
}

const char *blu2usb_profiles_target_name(blu2usb_mouse_target_t target)
{
    switch (target) {
    case BLU2USB_MOUSE_TARGET_LEFT: return "LEFT";
    case BLU2USB_MOUSE_TARGET_RIGHT: return "RIGHT";
    case BLU2USB_MOUSE_TARGET_MIDDLE: return "MIDDLE";
    case BLU2USB_MOUSE_TARGET_BACKWARD: return "BACKWARD";
    case BLU2USB_MOUSE_TARGET_FORWARD: return "FORWARD";
    case BLU2USB_MOUSE_TARGET_ESCAPE: return "ESCAPE"; /* legacy */
    case BLU2USB_MOUSE_TARGET_MODIFIER: return "MODIFIER";
    default: return "?";
    }
}
