#include "blu2usb/movement/movement.h"
#include <limits.h>

blu2usb_movement_settings_t blu2usb_movement_defaults(void)
{
    return (blu2usb_movement_settings_t){4u, 4u, 0u};
}

static bool allowed_gain(uint8_t value)
{
    return value == 1u || value == 2u || value == 3u || value == 4u ||
           value == 5u || value == 6u || value == 8u || value == 12u;
}

bool blu2usb_movement_valid(blu2usb_movement_settings_t settings)
{
    return allowed_gain(settings.horizontal_quarters) &&
           allowed_gain(settings.vertical_quarters) &&
           settings.rotation_quadrants < 4u;
}

void blu2usb_movement_pipeline_clear(blu2usb_movement_pipeline_t *pipeline)
{
    if (pipeline == NULL) return;
    pipeline->pending_x_quarters = 0;
    pipeline->pending_y_quarters = 0;
}

static int64_t safe_add(int64_t a, int64_t b)
{
    if (b > 0 && a > INT64_MAX - b) return INT64_MAX;
    if (b < 0 && a < INT64_MIN - b) return INT64_MIN;
    return a + b;
}

void blu2usb_movement_pipeline_push(blu2usb_movement_pipeline_t *pipeline,
                                    blu2usb_movement_settings_t settings,
                                    int32_t source_x, int32_t source_y)
{
    if (pipeline == NULL) return;
    if (!blu2usb_movement_valid(settings)) settings = blu2usb_movement_defaults();
    const int64_t x = source_x, y = source_y;
    int64_t rx = x, ry = y;
    switch (settings.rotation_quadrants) {
    case 1u: rx = y; ry = -x; break;
    case 2u: rx = -x; ry = -y; break;
    case 3u: rx = -y; ry = x; break;
    default: break;
    }
    pipeline->pending_x_quarters = safe_add(pipeline->pending_x_quarters,
                                           rx * settings.horizontal_quarters);
    pipeline->pending_y_quarters = safe_add(pipeline->pending_y_quarters,
                                           ry * settings.vertical_quarters);
}

static int8_t next_chunk(int64_t quarters)
{
    int64_t pixels = quarters / 4; /* truncation keeps subpixel remainder */
    if (pixels > INT8_MAX) pixels = INT8_MAX;
    if (pixels < INT8_MIN) pixels = INT8_MIN;
    return (int8_t)pixels;
}

void blu2usb_movement_pipeline_next(const blu2usb_movement_pipeline_t *pipeline,
                                    int8_t *usb_x, int8_t *usb_y)
{
    if (usb_x == NULL || usb_y == NULL) return;
    *usb_x = pipeline == NULL ? 0 : next_chunk(pipeline->pending_x_quarters);
    *usb_y = pipeline == NULL ? 0 : next_chunk(pipeline->pending_y_quarters);
}

void blu2usb_movement_pipeline_ack(blu2usb_movement_pipeline_t *pipeline,
                                   int8_t usb_x, int8_t usb_y)
{
    if (pipeline == NULL) return;
    pipeline->pending_x_quarters -= (int64_t)usb_x * 4;
    pipeline->pending_y_quarters -= (int64_t)usb_y * 4;
}
