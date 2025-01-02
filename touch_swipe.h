#ifndef TOUCH_SWIPE_H
#define TOUCH_SWIPE_H

#include <Arduino.h>
#include <lvgl.h>

/* 
   You already have a validate_touch(...) somewhere that returns 
   (x, y) if there's a valid touch. 
*/
bool validate_touch(lv_coord_t* x, lv_coord_t* y);
bool get_touch(lv_coord_t* x, lv_coord_t* y, bool print);

// Directions
typedef enum {
    SWIPE_DIR_NONE = 0,
    SWIPE_DIR_LEFT,
    SWIPE_DIR_RIGHT,
    SWIPE_DIR_UP,
    SWIPE_DIR_DOWN
} swipe_dir_t;

typedef enum {
    SWIPE_IDLE = 0,
    SWIPE_PRESSED,
    SWIPE_DRAGGING
} swipe_state_t;

typedef struct {
    swipe_state_t state;
    bool swipeDetected;
    swipe_dir_t swipeDir;
    lv_coord_t startX, startY;   // where press started
    lv_coord_t currentX, currentY;
    lv_coord_t lastGoodX, lastGoodY; // new fields
} swipe_tracker_t;

/**
 * @brief Non-blocking swipe detection that only requires the initial press 
 *        to be in the bounding box. Then user can drag outside.
 *
 * @param x_min, x_max, y_min, y_max  bounding area for initial press
 * @param min_swipe_length            min distance for a valid swipe
 * @param tracker                     pointer to a swipe_tracker_t object
 */
void update_swipe_state(int x_min, int x_max,
                        int y_min, int y_max,
                        int min_swipe_length,
                        swipe_tracker_t *tracker);

#endif // TOUCH_SWIPE_H
