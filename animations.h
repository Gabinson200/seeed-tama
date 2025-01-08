#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <lvgl.h>
#define USE_ARDUINO_GFX_LIBRARY
#include <Arduino.h>
#include "lv_xiao_round_screen.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"

typedef struct {
    lv_obj_t  *obj;
    lv_coord_t base_x;
    lv_coord_t base_y;
    uint16_t   index;  // store which layer number
    uint16_t   total_sprites; 
} pivot_sprite_t;

extern pivot_sprite_t g_sprites[];
extern uint16_t g_sprite_count;
extern lv_anim_t g_anim_storage[];

typedef void (*sprite_anim_cb_t)(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
);

typedef void (*sprite_exec_cb_t)(void *var, int32_t v);

void pet_anim(void * var, int32_t v);

void item_anim(void * var, int32_t v);

void rotate_anim(void * var, int32_t v);


void stack_anim(
    pivot_sprite_t    *sprites,
    uint16_t           sprite_count,
    int32_t           *current_angle,
    int32_t            end_angle_offset,
    uint32_t           duration,
    bool               infinite,
    sprite_exec_cb_t   exec_cb  // e.g. pet_anim
);

void swipe_anim(
    int                  x_min, 
    int                  x_max, 
    int                  y_min, 
    int                  y_max, 
    int                  min_swipe_length, 
    swipe_tracker_t     *tracker, 
    int32_t             *current_angle,

    // The array of sprites and how many
    pivot_sprite_t      *sprite_array,
    uint16_t             sprite_count,

    // Four callbacks (for left, right, up, down)
    sprite_anim_cb_t     left_animation,
    sprite_anim_cb_t     right_animation,
    sprite_anim_cb_t     up_animation,
    sprite_anim_cb_t     down_animation,

    // Animation offsets
    int32_t              left_offset, 
    int32_t              right_offset, 
    int32_t              up_offset, 
    int32_t              down_offset,

    // Common animation parameters
    uint32_t             duration,
    bool                 infinite
);

bool touch_anim(
    int x_min, int x_max, int y_min, int y_max,
    pivot_sprite_t *sprites,
    uint16_t       sprite_count,
    int32_t       *current_angle,
    sprite_anim_cb_t anim_func, // aggregator-level callback
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
);


// Wrapper functions to be actually called in main

void stack_anim_pet(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
);

void stack_anim_item(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
);


void stack_anim_rotate(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
);

#endif