#ifndef BLU2USB_MOVEMENT_MOVEMENT_H
#define BLU2USB_MOVEMENT_MOVEMENT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t horizontal_quarters; /* 1,2,3,4,5,6,8,12 */
    uint8_t vertical_quarters;   /* 1,2,3,4,5,6,8,12 */
    uint8_t rotation_quadrants;  /* counterclockwise: 0,90,180,270 */
} blu2usb_movement_settings_t;

typedef struct {
    int64_t pending_x_quarters;
    int64_t pending_y_quarters;
} blu2usb_movement_pipeline_t;

blu2usb_movement_settings_t blu2usb_movement_defaults(void);
bool blu2usb_movement_valid(blu2usb_movement_settings_t settings);
/* Pure, pre-gain coordinate transform; same canonical orientation as USB cursor. */
void blu2usb_movement_rotate_delta(blu2usb_movement_settings_t settings,
                                   int32_t source_x, int32_t source_y,
                                   int64_t *rotated_x, int64_t *rotated_y);
void blu2usb_movement_pipeline_clear(blu2usb_movement_pipeline_t *pipeline);
void blu2usb_movement_pipeline_push(blu2usb_movement_pipeline_t *pipeline,
                                    blu2usb_movement_settings_t settings,
                                    int32_t source_x, int32_t source_y);
void blu2usb_movement_pipeline_next(const blu2usb_movement_pipeline_t *pipeline,
                                    int8_t *usb_x, int8_t *usb_y);
void blu2usb_movement_pipeline_ack(blu2usb_movement_pipeline_t *pipeline,
                                   int8_t usb_x, int8_t usb_y);
#endif
