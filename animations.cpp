#include "animations.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"

void set_angle(void * var, int32_t v) {
    pivot_sprite_t  * sq_data = (pivot_sprite_t *)var;
    lv_obj_t        * obj     = sq_data->obj;

    // Rotate the sprite
    lv_img_set_angle(obj, v);

    // Get the sprite index (layer number)
    uint16_t sprite_index = sq_data - g_sprites;

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

void spin_stack(int32_t *current_angle, int32_t end_angle_offset, uint32_t duration, bool infinite) {
    int32_t start_angle = *current_angle; // Start from the current angle passed in
    int32_t end_angle = *current_angle + end_angle_offset; // Calculate the new end angle

    for (uint16_t i = 0; i < g_sprite_count; i++) {
        lv_anim_del(g_sprites[i].obj, NULL); // Stop ongoing animations
        lv_anim_init(&g_anim_storage[i]);
        lv_anim_set_var(&g_anim_storage[i], &g_sprites[i]);
        lv_anim_set_exec_cb(&g_anim_storage[i], set_angle);
        lv_anim_set_time(&g_anim_storage[i], duration);
        lv_anim_set_values(&g_anim_storage[i], start_angle, end_angle);
        if (infinite) {
            lv_anim_set_repeat_count(&g_anim_storage[i], LV_ANIM_REPEAT_INFINITE);
        }
        lv_anim_start(&g_anim_storage[i]);
    }

    *current_angle = end_angle; // Update the current angle for the animation
}

void swipe_anim(int x_min, int x_max, int y_min, int y_max, int min_swipe_length, swipe_tracker_t *tracker, int32_t *current_angle, 
                void (*left_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*right_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*up_animation)(int32_t *, int32_t, uint32_t, bool),
                void (*down_animation)(int32_t *, int32_t, uint32_t, bool),
                int32_t left_offset, int32_t right_offset, int32_t up_offset, int32_t down_offset,
                uint32_t duration, bool infinite) {
    // Update the swipe state based on touch inputs
    update_swipe_state(x_min, x_max, y_min, y_max, min_swipe_length, tracker);

    // Check if a swipe was detected
    if (tracker->swipeDetected) {
        Serial.print("Detected swipe direction: ");
        switch (tracker->swipeDir) {
            case SWIPE_DIR_LEFT:
                Serial.println("LEFT");
                left_animation(current_angle, left_offset, duration, infinite);
                break;
            case SWIPE_DIR_RIGHT:
                Serial.println("RIGHT");
                right_animation(current_angle, right_offset, duration, infinite);
                break;
            case SWIPE_DIR_UP:
                Serial.println("UP");
                up_animation(current_angle, up_offset, duration, infinite);
                break;
            case SWIPE_DIR_DOWN:
                Serial.println("DOWN");
                down_animation(current_angle, down_offset, duration, infinite);
                break;
            default:
                Serial.println("NONE");
                break;
        }

        tracker->swipeDetected = false; // Reset swipe detection
    }
    delay(10);
}