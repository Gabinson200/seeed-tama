#include "touch_swipe.h"

// Helper function to compute direction
static swipe_dir_t compute_swipe_dir(lv_coord_t startX, lv_coord_t startY,
                                     lv_coord_t endX,   lv_coord_t endY,
                                     int min_swipe_length)
{
    int dx = endX - startX;
    int dy = endY - startY;
    int absX = abs(dx);
    int absY = abs(dy);

    // If both movements are below threshold, no real swipe
    if (absX < min_swipe_length && absY < min_swipe_length) {
        return SWIPE_DIR_NONE;
    }

    // Decide which axis is dominant
    if (absX > absY) {
        return (dx < 0) ? SWIPE_DIR_LEFT : SWIPE_DIR_RIGHT;
    } else {
        return (dy < 0) ? SWIPE_DIR_UP : SWIPE_DIR_DOWN;
    }
}

// Optional: define a threshold for ignoring large jumps
static const int MAX_JUMP = 80; // or your chosen threshold

void update_swipe_state(int x_min, int x_max,
                        int y_min, int y_max,
                        int min_swipe_length,
                        swipe_tracker_t *tracker)
{
    // Clear any detection from previous loop
    tracker->swipeDetected = false;

    // Get current touch status
    lv_coord_t x, y;
    bool isTouchValid = validate_touch(&x, &y);

    // We can keep a small static counter to confirm user REALLY released
    // rather than a brief glitch
    static uint8_t releaseCounter = 0;

    switch (tracker->state) 
    {
    case SWIPE_IDLE:
    {
        releaseCounter = 0; // Reset on new cycle
        if (isTouchValid) {
            // Check if initial press is inside the bounding box
            if ((x >= x_min && x <= x_max) && (y >= y_min && y <= y_max)) {
                // Save the start location
                tracker->startX = x;
                tracker->startY = y;
                tracker->currentX = x;
                tracker->currentY = y;
                tracker->lastGoodX = x;  // Keep a "last good" backup
                tracker->lastGoodY = y;  
                tracker->state = SWIPE_PRESSED;
                Serial.println("SWIPE_IDLE -> SWIPE_PRESSED");
            }
        }
        break;
    }

    case SWIPE_PRESSED:
    {
        releaseCounter = 0;
        if (isTouchValid) {
            // If user is still pressing, we consider them dragging now
            // We'll move to DRAGGING next iteration
            tracker->currentX = x;
            tracker->currentY = y;
            tracker->lastGoodX = x;
            tracker->lastGoodY = y;

            // A small check to skip if movement is tiny vs. start?
            // (optional) -> We'll just proceed to DRAGGING
            tracker->state = SWIPE_DRAGGING;
            Serial.println("SWIPE_PRESSED -> SWIPE_DRAGGING");
        } else {
            // User lifted too soon => no swipe
            tracker->state = SWIPE_IDLE;
            Serial.println("SWIPE_PRESSED -> SWIPE_IDLE (released quickly)");
        }
        break;
    }

    case SWIPE_DRAGGING:
    {
        if (isTouchValid) {
            releaseCounter = 0;

            // Optionally ignore huge jumps:
            int jumpX = abs(x - tracker->lastGoodX);
            int jumpY = abs(y - tracker->lastGoodY);

            // If the new reading is drastically different from last good,
            // ignore it to avoid spurious (140,0) style data
            if (jumpX > MAX_JUMP || jumpY > MAX_JUMP) {
                Serial.print("Ignoring outlier: (");
                Serial.print(x);
                Serial.print(", ");
                Serial.print(y);
                Serial.println(")");
                // do NOT update currentX/currentY
            } else {
                // Normal updates
                tracker->currentX = x;
                tracker->currentY = y;
                tracker->lastGoodX = x;
                tracker->lastGoodY = y;
            }

        } else {
            // If not valid, user might be releasing
            releaseCounter++;
            if (releaseCounter >= 2) {
                // Actually finalize the swipe after 2 consecutive invalid reads
                lv_coord_t endX = tracker->currentX;
                lv_coord_t endY = tracker->currentY;

                // Debug print
                Serial.print("Start: (");
                Serial.print(tracker->startX);
                Serial.print(",");
                Serial.print(tracker->startY);
                Serial.print(") End: (");
                Serial.print(endX);
                Serial.print(",");
                Serial.print(endY);
                Serial.print(") => dx=");
                Serial.print(endX - tracker->startX);
                Serial.print(", dy=");
                Serial.println(endY - tracker->startY);

                swipe_dir_t dir = compute_swipe_dir(tracker->startX, tracker->startY,
                                                    endX, endY,
                                                    min_swipe_length);

                if (dir != SWIPE_DIR_NONE) {
                    tracker->swipeDetected = true;
                    tracker->swipeDir = dir;
                    Serial.print("SWIPE_DRAGGING -> SWIPE_IDLE, swipeDetected in direction: ");
                    switch (dir) {
                        case SWIPE_DIR_LEFT:  Serial.println("LEFT");  break;
                        case SWIPE_DIR_RIGHT: Serial.println("RIGHT"); break;
                        case SWIPE_DIR_UP:    Serial.println("UP");    break;
                        case SWIPE_DIR_DOWN:  Serial.println("DOWN");  break;
                        default:              Serial.println("NONE?"); break;
                    }
                } else {
                    Serial.println("SWIPE_DRAGGING -> SWIPE_IDLE, short swipe");
                }
                tracker->state = SWIPE_IDLE;
                releaseCounter = 0;
            }
        }
        break;
    }
    }
}
