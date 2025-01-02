// -----------------------------------------
//         Header guards and prototypes
// -----------------------------------------
#ifndef TOUCH_SENSOR_FUNCTIONS_H
#define TOUCH_SENSOR_FUNCTIONS_H

#include <lvgl.h>
#define USE_ARDUINO_GFX_LIBRARY

// Display/Touch init (from lv_xiao_round_screen.h)
void lv_xiao_disp_init(void);
void lv_xiao_touch_init(void);

// Bounds checking
bool is_within_square_bounds(int x, int y, int x_min, int x_max, int y_min, int y_max);
bool is_within_square_bounds_center(int x, int y, int center_x, int center_y, int half_width, int half_height);
bool is_within_circle_bounds(int x, int y, int center_x, int center_y, int radius);

// Drawing function
void draw_area(lv_area_t area, bool is_circle, bool clear_previous);
static void shape_event_cb(lv_event_t * e);

// Touch functions
bool validate_touch(lv_coord_t* touchX, lv_coord_t* touchY);
bool get_touch(lv_coord_t* x, lv_coord_t* y, bool print = false);

// Checking if user touched certain bounds
bool get_touch_in_area(int x_min, int x_max, int y_min, int y_max, bool view = false);
bool get_touch_in_area_center(int center_x, int center_y, int half_width, int half_height, bool view = false);
bool get_touch_in_area_circle(int center_x, int center_y, int radius, bool view = false);

// Press (hold) functions
bool pressed(int duration, int x_min, int x_max, int y_min, int y_max, bool view = false);
bool pressed_center(int duration, int center_x, int center_y, int half_width, int half_height, bool view = false);
bool pressed_circle(int duration, int center_x, int center_y, int radius, bool view = false);



#endif