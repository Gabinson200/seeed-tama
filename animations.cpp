#include "animations.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"

#define MAX_SPRITES 20

// This function applied all animations to all layers of a sprite-stack
void stack_anim(
    pivot_sprite_t    *sprites,
    uint16_t           sprite_count,
    int32_t           *current_angle,
    int32_t            end_angle_offset,
    uint32_t           duration,
    bool               infinite,
    sprite_exec_cb_t   exec_cb  // e.g. pet_anim
)
{
    int32_t start_angle = *current_angle;
    int32_t end_angle   = start_angle + end_angle_offset;

    // Static to avoid going out of scope (or you can make it global)
    static lv_anim_t local_anim_storage[MAX_SPRITES];

    // For each sprite in the stack...
    for (uint16_t i = 0; i < sprite_count && i < MAX_SPRITES; i++) {

        // Stop any ongoing animations on this sprite
        lv_anim_del(sprites[i].obj, NULL);

        // Initialize an animation
        lv_anim_init(&local_anim_storage[i]);

        // "var" is the pivot_sprite_t pointer for this layer
        lv_anim_set_var(&local_anim_storage[i], &sprites[i]);
        // This is the CRUCIAL part:
        lv_anim_set_exec_cb(&local_anim_storage[i], exec_cb);

        lv_anim_set_time(&local_anim_storage[i], duration);
        lv_anim_set_values(&local_anim_storage[i], start_angle, end_angle);
        if (infinite) {
            lv_anim_set_repeat_count(&local_anim_storage[i], LV_ANIM_REPEAT_INFINITE);
        }

        // Kick it off
        lv_anim_start(&local_anim_storage[i]);
    }

    // Update angle for next time (optional)
    *current_angle = end_angle;
}

//#####################################################################################
//-------- Actual animation definitions
//#####################################################################################

void pet_anim(void * var, int32_t v) {
    pivot_sprite_t  * sq_data = (pivot_sprite_t *)var;
    lv_obj_t        * obj     = sq_data->obj;

    // Rotate the sprite
    lv_img_set_angle(obj, v);

    // Get the sprite index (layer number)
    uint16_t sprite_index = sq_data->index;

    // Get the image dimensions dynamically
    const lv_img_dsc_t *img_dsc = (const lv_img_dsc_t *)lv_img_get_src(obj);
    lv_coord_t img_width = img_dsc->header.w; // Image width
    lv_coord_t img_height = img_dsc->header.h; // Image height

    int32_t pivot_offset_y = sinf((v / 3600.0) * M_PI * 2) * sprite_index / 4;
    lv_img_set_pivot(obj, img_width / 2, img_height / 2 - pivot_offset_y); // Adjusted pivot

    // Position
    lv_coord_t x_pos = sq_data->base_x - (img_width / 2);
    float layer_multiplier = 1.0 + (sprite_index * 1.2); // increasing change in y offset
    lv_coord_t y_pos = sq_data->base_y - (layer_multiplier * 2);
    lv_obj_set_pos(obj, x_pos, y_pos);
}

void item_anim(void * var, int32_t v) {
    pivot_sprite_t *sq_data = (pivot_sprite_t *)var;
    lv_obj_t       *obj     = sq_data->obj;

    // 1) Spin each sprite by "v" degrees (LVGL uses 0..3600 = 0..360 deg).
    lv_img_set_angle(obj, v);

    // 2) Get sprite index (0..total_sprites-1) and total
    uint16_t sprite_index   = sq_data->index;
    uint16_t total_sprites  = sq_data->total_sprites;

    // 3) Basic image info
    const lv_img_dsc_t *img_dsc = (const lv_img_dsc_t *)lv_img_get_src(obj);
    lv_coord_t img_width  = img_dsc->header.w;
    lv_coord_t img_height = img_dsc->header.h;

    // Center pivot so rotation is about image center
    lv_img_set_pivot(obj, img_width / 2, img_height / 2);

    // Ensure v wraps around every full rotation to avoid precision issues
    int32_t wrapped_v = v % 3600; // Always between 0 and 3600
    float angle_f = wrapped_v / 3600.0f; // Normalize to [0, 1]

    // ENTIRE STACK RISE/FALL (Positive Only)
    // Sin wave shifted and scaled to [0, 1]
    float amplitude_rise = 10.0f;
    float entire_stack_offset = (sinf(angle_f * 2.0f * M_PI) + 1.0f) * 0.5f * amplitude_rise;

    // PER-LAYER OFFSET
    // Sin wave already in [0, 1] range; no need to shift
    float wave = sinf(angle_f * M_PI);
    float amplitude_layer = 3.0f;
    float baseline_offset = 2.0f;       // Offset layers at the start and end
    float layer_offset = baseline_offset * sprite_index + (wave * amplitude_layer * sprite_index);

    // FINAL POSITION
    lv_coord_t x_pos = sq_data->base_x - (img_width / 2);
    lv_coord_t y_pos = sq_data->base_y 
                    - (lv_coord_t)entire_stack_offset 
                    - (lv_coord_t)layer_offset;

    lv_obj_set_pos(obj, x_pos, y_pos);
}

void rotate_anim(void * var, int32_t v){
    pivot_sprite_t  * sq_data = (pivot_sprite_t *)var;
    lv_obj_t        * obj     = sq_data->obj;

    // 1) Spin each sprite by "v" degrees (LVGL uses 0..3600 = 0..360 deg).
    lv_img_set_angle(obj, v);

    // 2) Get sprite index (0..total_sprites-1) and total
    uint16_t sprite_index   = sq_data->index;
    uint16_t total_sprites  = sq_data->total_sprites;

    // 3) Basic image info
    const lv_img_dsc_t *img_dsc = (const lv_img_dsc_t *)lv_img_get_src(obj);
    lv_coord_t img_width  = img_dsc->header.w;
    lv_coord_t img_height = img_dsc->header.h;

    // Center pivot so rotation is about image center
    lv_img_set_pivot(obj, img_width / 2, img_height / 2);

    float baseline_offset = 1.0f;  

    // FINAL POSITION
    lv_coord_t x_pos = sq_data->base_x - (img_width / 2);
    lv_coord_t y_pos = sq_data->base_y 
                    -baseline_offset* sprite_index;

    lv_obj_set_pos(obj, x_pos, y_pos);
}

//#####################################################################################
//-------- Functions that play specific animations on different user inputs
//#####################################################################################


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
) {
    // Update the swipe state based on touch inputs
    update_swipe_state(x_min, x_max, y_min, y_max, min_swipe_length, tracker);

    // Check if a swipe was detected
    if (tracker->swipeDetected) {
        Serial.print("Detected swipe direction: ");
        switch (tracker->swipeDir) {
            case SWIPE_DIR_LEFT:
                Serial.println("LEFT");
                left_animation(sprite_array, sprite_count, current_angle, left_offset, duration, infinite);
                break;
            case SWIPE_DIR_RIGHT:
                Serial.println("RIGHT");
                right_animation(sprite_array, sprite_count, current_angle, right_offset, duration, infinite);
                break;
            case SWIPE_DIR_UP:
                Serial.println("UP");
                up_animation(sprite_array, sprite_count, current_angle, up_offset, duration, infinite);
                break;
            case SWIPE_DIR_DOWN:
                Serial.println("DOWN");
                down_animation(sprite_array, sprite_count, current_angle, down_offset, duration, infinite);
                break;
            default:
                Serial.println("NONE");
                break;
        }

        tracker->swipeDetected = false; // Reset swipe detection
    }
    delay(10);
}


bool touch_anim(
    int x_min, int x_max, int y_min, int y_max,
    pivot_sprite_t *sprites,
    uint16_t       sprite_count,
    int32_t       *current_angle,
    sprite_anim_cb_t anim_func, // aggregator-level callback
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
)
{
    if (get_touch_in_area(x_min, x_max, y_min, y_max, false)) {
        // We call anim_func(...) if touched
        anim_func(sprites, sprite_count, current_angle,
                  end_angle_offset, duration, infinite);
        return true;
    }
    return false;
}


//#####################################################################################
//-------- Wrapper functions that pass specific animations to the stack_anim funciton
//#####################################################################################

// The aggregator-level function pointer signature from your existing code:
typedef void (*sprite_anim_cb_t)(
    pivot_sprite_t *,
    uint16_t,
    int32_t *,
    int32_t,
    uint32_t,
    bool
);

// Define a wrapper that calls stack_anim with pet_anim as the exec callback:
void stack_anim_pet(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
)
{
    stack_anim(sprites,
               sprite_count,
               current_angle,
               end_angle_offset,
               duration,
               infinite,
               pet_anim  // <--- pass your custom per-sprite function
    );
}


void stack_anim_item(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
){

  stack_anim(sprites,
               sprite_count,
               current_angle,
               end_angle_offset,
               duration,
               infinite,
               item_anim  // <--- pass your custom per-sprite function
    );
}

void stack_anim_rotate(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite
){

  stack_anim(sprites,
               sprite_count,
               current_angle,
               end_angle_offset,
               duration,
               infinite,
               rotate_anim  // <--- pass your custom per-sprite function
    );
}
