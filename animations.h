#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <lvgl.h>
#define USE_ARDUINO_GFX_LIBRARY
#include <Arduino.h>
#include "touch_swipe.h"

typedef struct {
    lv_obj_t  * obj;
    lv_coord_t base_x;  // X position of the sprite stack
    lv_coord_t base_y;  // Y position of the sprite stack
} pivot_sprite_t;

extern pivot_sprite_t g_sprites[];
extern uint16_t g_sprite_count;
extern lv_anim_t g_anim_storage[];


void set_angle(void * var, int32_t v);

void spin_stack(int32_t *current_angle, int32_t end_angle_offset, uint32_t duration, bool infinite);

void swipe_anim(int x_min, int x_max, int y_min, int y_max, int min_swipe_length, swipe_tracker_t *tracker, int32_t *current_angle, 
                void (*left_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*right_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*up_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*down_animation)(int32_t *, int32_t, uint32_t, bool),
                int32_t left_offset, int32_t right_offset, int32_t up_offset, int32_t down_offset,
                uint32_t duration, bool infinite);

#endif