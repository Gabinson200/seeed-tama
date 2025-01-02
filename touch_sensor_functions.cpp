#include "touch_sensor_functions.h"
#define USE_ARDUINO_GFX_LIBRARY // make sure this goes before xiao round screen lib
#include "lv_xiao_round_screen.h"
#include <Arduino.h>
#include <math.h>

//------------------- Boundary checks ------------------------

// Base function for square bounds check using corner coordinates
bool is_within_square_bounds(int x, int y, int x_min, int x_max, int y_min, int y_max) {
    return (x >= x_min && x <= x_max && y >= y_min && y <= y_max);
}

// Overloaded function that uses the base square bounds check
bool is_within_square_bounds_center(int x, int y, int center_x, int center_y, int half_width, int half_height) {
    return is_within_square_bounds(x, y,
        center_x - half_width, center_x + half_width,
        center_y - half_height, center_y + half_height);
}

bool is_within_circle_bounds(int x, int y, int center_x, int center_y, int radius) {
    int dx = x - center_x;
    int dy = y - center_y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

//----------------------- Touch Validation ------------------------

bool validate_touch(lv_coord_t* touchX, lv_coord_t* touchY) {
    if (!chsc6x_is_pressed()) {
        return false; 
    }
    // Read coords
    chsc6x_get_xy(touchX, touchY);

    // Debug printing
    Serial.print("validate_touch -> raw coords: (");
    Serial.print(*touchX); 
    Serial.print(", "); 
    Serial.print(*touchY);
    Serial.println(")");

    // Instead of rejecting out-of-bounds, clamp them:
    if (*touchX > 240) *touchX = 240;
    if (*touchY > 240) *touchY = 240;
    
    // Possibly clamp to 0 as well, if negative values appear:
    // if (*touchX < 0) *touchX = 0;
    // if (*touchY < 0) *touchY = 0;

    return true;
}

//----------------------- Touches -------------------------------


bool get_touch(lv_coord_t* x, lv_coord_t* y, bool print) {
  lv_coord_t touchX, touchY;

  if (chsc6x_is_pressed()) {
    chsc6x_get_xy(&touchX, &touchY);
    if (touchX > 240 || touchY > 240) {
      touchX = 0;
      touchY = 0;
    }
    if(print){
      Serial.print("Touch coordinates: X = ");
      Serial.print(touchX);
      Serial.print(", Y = ");
      Serial.println(touchY);
    }
    return true;
  }
  return false;
}

static lv_obj_t *last_shape = NULL;  // Keep track of last created shape

void draw_area(lv_area_t area, bool is_circle, bool clear_previous) {
    // Clear previous shape if requested
    if (clear_previous && last_shape != NULL) {
        lv_obj_del(last_shape);
        last_shape = NULL;
    }

    // Create the shape based on type
    if (is_circle) {
        // Create a circle using an object with rounded corners
        lv_obj_t *circle = lv_obj_create(lv_scr_act());
        int radius = (area.x2 - area.x1) / 2;  // Assuming width = height for circle
        
        lv_obj_set_size(circle, radius * 2, radius * 2);
        lv_obj_set_pos(circle, area.x1, area.y1);
        
        // Make it fully rounded
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
        
        // Set other styles
        lv_obj_set_style_bg_color(circle, lv_color_hex(0x0000FF), 0);  // Blue color
        lv_obj_set_style_bg_opa(circle, LV_OPA_50, 0);
        lv_obj_set_style_border_width(circle, 0, 0);
        
        last_shape = circle;
    } else {
        // Create a rectangle
        lv_obj_t *rect = lv_obj_create(lv_scr_act());
        
        lv_obj_set_size(rect, area.x2 - area.x1, area.y2 - area.y1);
        lv_obj_set_pos(rect, area.x1, area.y1);
        
        // Set styles
        lv_obj_set_style_radius(rect, 0, 0);  // Sharp corners
        lv_obj_set_style_bg_color(rect, lv_color_hex(0x0000FF), 0);  // Blue color
        lv_obj_set_style_bg_opa(rect, LV_OPA_50, 0);
        lv_obj_set_style_border_width(rect, 0, 0);
        
        last_shape = rect;
    }

    // Add click event handler
    lv_obj_add_event_cb(last_shape, shape_event_cb, LV_EVENT_CLICKED, NULL);
}

// Event callback for shape clicks
static void shape_event_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    bool is_circle = (lv_obj_get_style_radius(obj, 0) == LV_RADIUS_CIRCLE);
    Serial.print("Touch detected on ");
    Serial.println(is_circle ? "circle" : "rectangle");
}

bool get_touch_in_area(int x_min, int x_max, int y_min, int y_max, bool view) {
    lv_coord_t touchX, touchY;

    if (view) {
        lv_area_t area = {x_min, y_min, x_max, y_max};
        draw_area(area, false, true);  // false for rectangle, true to clear previous
    }

    return validate_touch(&touchX, &touchY) &&
           is_within_square_bounds(touchX, touchY, x_min, x_max, y_min, y_max);
}

// Overloaded function for center-based rectangular area check
bool get_touch_in_area_center(int center_x, int center_y, int half_width, int half_height, bool view) {
    return get_touch_in_area(
        center_x - half_width, center_x + half_width,
        center_y - half_height, center_y + half_height, view
    );
}

bool get_touch_in_area_circle(int center_x, int center_y, int radius, bool view) {
    lv_coord_t touchX, touchY;

    if (view) {
        lv_area_t area = {
            center_x - radius, 
            center_y - radius,
            center_x + radius, 
            center_y + radius
        };
        draw_area(area, true, true);  // true for circle, true to clear previous
    }

    return validate_touch(&touchX, &touchY) &&
           is_within_circle_bounds(touchX, touchY, center_x, center_y, radius);
}

//------------------- Press Handling --------------------------

// Helper function to wait for touch release
void wait_for_release() {
    while (chsc6x_is_pressed()) {
        delay(10);
    }
}

// Helper function to handle press timing
bool handle_press_timing(bool isInArea, unsigned long& pressStart, bool& isPressed) {
    if (isInArea) {
        if (!isPressed) {
            pressStart = millis();
            isPressed = true;
        }
        return true;
    }
    isPressed = false;
    return false;
}

/* 
 *  pressed(...) 
 * 
 *  Blocks in a 'while(true)' loop until either:
 *   1) The user presses in the specified area for 'duration' ms (returns true), or 
 *   2) The user releases or moves out of the area before that time (returns false).
 */
bool pressed(int duration, int x_min, int x_max, int y_min, int y_max, bool view) {
    lv_coord_t touchX, touchY;
    unsigned long pressStart = 0;
    bool isPressed = false;

    if (view) {
        lv_area_t area = {x_min, y_min, x_max, y_max};
        draw_area(area, false, true);
    }

    // Blocking loop
    while (true) {
        // Check for valid touch
        if (validate_touch(&touchX, &touchY)) {
            bool isInArea = is_within_square_bounds(touchX, touchY, x_min, x_max, y_min, y_max);

            if (isInArea) {
                // If not previously pressed, mark a new press start time
                if (!isPressed) {
                    pressStart = millis();
                    isPressed  = true;
                }

                // If enough time has elapsed, success
                if (millis() - pressStart >= (unsigned long)duration) {
                    return true;
                }
            }
            else {
                // Touched outside the area => reset
                isPressed  = false;
                pressStart = 0;
            }
        }
        else {
            // No valid touch -> if we were tracking a press, it means user released
            if (isPressed) {
                return false;
            }
            delay(10); // Avoid busy waiting
        }
    }
}

bool pressed_center(int duration, int center_x, int center_y, int half_width, int half_height, bool view) {
    return pressed(duration,
                   center_x - half_width, center_x + half_width,
                   center_y - half_height, center_y + half_height, view);
}

bool pressed_circle(int duration, int center_x, int center_y, int radius, bool view) {
    lv_coord_t touchX, touchY;
    unsigned long pressStart = 0;
    bool isPressed = false;

    if (view) {
        lv_area_t area = {
            center_x - radius, center_y - radius,
            center_x + radius, center_y + radius
        };
        draw_area(area, true, true);
    }

    // Blocking loop
    while (true) {
        if (validate_touch(&touchX, &touchY)) {
            bool isInArea = is_within_circle_bounds(touchX, touchY, center_x, center_y, radius);

            if (isInArea) {
                if (!isPressed) {
                    pressStart = millis();
                    isPressed  = true;
                }
                if (millis() - pressStart >= (unsigned long)duration) {
                    return true;
                }
            }
            else {
                isPressed  = false;
                pressStart = 0;
            }
        }
        else {
            // Released or invalid => if previously pressed, user let go
            if (isPressed) {
                return false;
            }
            delay(10);
        }
    }
}




