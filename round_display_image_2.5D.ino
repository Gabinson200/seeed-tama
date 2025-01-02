#define USE_ARDUINO_GFX_LIBRARY
#include <lvgl.h>
#include "lv_xiao_round_screen.h"
#include "dino_sprites.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"

// Example image declarations
LV_IMG_DECLARE(tile000);
LV_IMG_DECLARE(tile001);
LV_IMG_DECLARE(tile002);
LV_IMG_DECLARE(tile003);
LV_IMG_DECLARE(tile004);
LV_IMG_DECLARE(tile005);
LV_IMG_DECLARE(tile006);
LV_IMG_DECLARE(tile007);
LV_IMG_DECLARE(tile008);
LV_IMG_DECLARE(tile009);
LV_IMG_DECLARE(tile010);
LV_IMG_DECLARE(tile011);
LV_IMG_DECLARE(tile012);
LV_IMG_DECLARE(tile013);
LV_IMG_DECLARE(tile014);

static const lv_img_dsc_t *sprite_images[] = {
    &tile000, &tile001, &tile002, &tile003, &tile004,
    &tile005, &tile006, &tile007, &tile008, &tile009,
    &tile010, &tile011, &tile012, &tile013, &tile014
};

typedef struct {
    lv_obj_t  * obj;
    lv_coord_t base_x;  // X position of the sprite stack
    lv_coord_t base_y;  // Y position of the sprite stack
} pivot_sprite_t;

static swipe_tracker_t g_swipeTracker = {
    SWIPE_IDLE, false, SWIPE_DIR_NONE, 
    0,0, 0,0, 0,0  // initializing startX, startY, currentX, currentY, lastGoodX, lastGoodY
};

// We'll store up to 20 sprites
static pivot_sprite_t g_sprites[20];
static lv_anim_t      g_anim_storage[20];
static uint16_t       g_sprite_count = 0;

static bool rotating = false; // Flag to control animation
static int32_t current_angle = 0; // Current rotation angle

// Sprite stack area (define bounds)
static const lv_coord_t SPRITE_AREA_X = 120 - 32; // img initial x - img width / 2
static const lv_coord_t SPRITE_AREA_Y = 120 - 32;
static const lv_coord_t SPRITE_AREA_WIDTH = 64;
static const lv_coord_t SPRITE_AREA_HEIGHT = 64;

// Touch positions
bool valid_touch = false;
static lv_coord_t touch_start_x = 0;
static lv_coord_t touch_start_y = 0;


// ---------------------------------------------------------
//  Animation callbacks
// ---------------------------------------------------------
static void set_angle(void * var, int32_t v) {
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

// Function to spin the sprite stack
static void spin_stack(int32_t start_angle, int32_t end_angle, uint32_t duration, bool infinite) {
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
}

// Check if touch is within the sprite stack area
static bool is_within_sprite_area(lv_coord_t x, lv_coord_t y) {
    return (x >= SPRITE_AREA_X && x <= (SPRITE_AREA_X + SPRITE_AREA_WIDTH) &&
            y >= SPRITE_AREA_Y && y <= (SPRITE_AREA_Y + SPRITE_AREA_HEIGHT));
}

// Touch and gesture event handler
static void touch_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        // Store touch start position
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);
        touch_start_x = point.x;
        touch_start_y = point.y;

        // Validate if the touch started in the sprite area
        valid_touch = is_within_sprite_area(touch_start_x, touch_start_y);
        if (valid_touch) {
            Serial.println("Touch Pressed inside sprite area");
        }
    }  
    
    if (code == LV_EVENT_GESTURE && valid_touch) {
        // Handle swipe gestures only if touch started inside the sprite area
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
            Serial.println("Swipe Left Detected");
            spin_stack(current_angle, current_angle - 200, 300, false); // Rotate left
            current_angle -= 200;
        } else if (dir == LV_DIR_RIGHT) {
            Serial.println("Swipe Right Detected");
            spin_stack(current_angle, current_angle + 200, 300, false); // Rotate right
            current_angle += 200;
        }
        if (code == LV_EVENT_RELEASED) {
            // Reset validation on touch release
            valid_touch = false;
            Serial.println("Touch Released");
        }
    }
}


// ---------------------------------------------------------
//  Create sprites with pseudo-3D effect
// ---------------------------------------------------------
void create_sprites_pseudo_3d(uint16_t num_sprites,
                              lv_coord_t base_x,
                              lv_coord_t base_y,
                              lv_coord_t spacing_y,
                              uint16_t zoom_step)
{
    if (num_sprites > 20) num_sprites = 20;
    g_sprite_count = num_sprites;

    uint16_t base_zoom = 256; // default 1:1 scale

    for (uint16_t i = 0; i < num_sprites; i++) {
        lv_obj_t *sprite_img = lv_img_create(lv_scr_act());
        if (!sprite_img) {
            Serial.println("Failed to create image object");
            continue;
        }

        // Set the image source
        const lv_img_dsc_t *src = sprite_images[i % num_sprites];
        lv_img_set_src(sprite_img, src);

        // Set zoom to create pseudo-3D layering
        uint16_t current_zoom = base_zoom + (i * zoom_step);
        lv_img_set_zoom(sprite_img, current_zoom);

        // Calculate positions
        lv_coord_t x_pos = base_x - 32; // Base x location provided by the user
        lv_coord_t y_pos = base_y - (i * spacing_y); // Adjust y position based on layer index

        lv_obj_set_pos(sprite_img, x_pos, y_pos);

        // Store
        g_sprites[i].obj = sprite_img;
        g_sprites[i].base_x = base_x;
        g_sprites[i].base_y = base_y;

        // Initialize the sprite with a static angle (no animation)
        lv_img_set_angle(sprite_img, 0); // Start with no rotation
    }

    // Attach event callbacks for the screen
    //lv_obj_add_event_cb(lv_scr_act(), touch_event_cb, LV_EVENT_PRESSED, NULL);
    //lv_obj_add_event_cb(lv_scr_act(), touch_event_cb, LV_EVENT_GESTURE, NULL);
    //lv_obj_add_event_cb(lv_scr_act(), touch_event_cb, LV_EVENT_RELEASED, NULL);
}

// ---------------------------------------------------------
//  LVGL Setup / Loop
// ---------------------------------------------------------

#define SPRITE_W   64
#define SPRITE_H   64
#define BUFF_SIZE (SPRITE_W * SPRITE_H)

static lv_color_t buf1[BUFF_SIZE];
static lv_color_t buf2[BUFF_SIZE];

void setup() {
    Serial.begin(115200);
    delay(100); // Give time for Serial to initialize
    
    Serial.println("Starting initialization...");

    // 1) Initialize LVGL
    lv_init();
    Serial.println("LVGL initialized");

    // 2) Initialize the display and touch
    lv_xiao_disp_init();
    Serial.println("Display initialized");
    
    lv_xiao_touch_init();
    Serial.println("Touch initialized");

    // Create our rotating sprites
    create_sprites_pseudo_3d(/*num_sprites=*/15, /*base_x=*/120, /*base_y=*/90, /*spacing_y=*/1, /*zoom_step=*/0);
    Serial.println("Created sprites");

    Serial.println("Setup complete");
}

// Pseudocode
void loop() {
  /*
    // If user touches in the sprite area, spin
    if (get_touch_in_area(
            SPRITE_AREA_X,
            SPRITE_AREA_X + SPRITE_AREA_WIDTH,
            SPRITE_AREA_Y,
            SPRITE_AREA_Y + SPRITE_AREA_HEIGHT,
            false // 'view' param -> whether to draw a highlight shape
        ))
    {
        Serial.println("Sprite area touched -> Spin!");
        spin_stack(current_angle, current_angle + 200, 300, false);
        current_angle += 200;
    }
    */


    /*
    // Suppose you want to detect a LEFT swipe from (100,120)-(140,160)
    if (swipe_in_area(100,140,20,60, SWIPE_DIR_LEFT, 30, true)) {
        Serial.println("Swiped left inside the area with length >= 30");
    }

    // Or detect any direction from a center-based area
    if (swipe_in_area_center(120, 200, 20, 20, SWIPE_DIR_ANY, 50, true)) {
        Serial.println("Swiped in any direction within center area (50px min length)!");
    }
    */

    update_swipe_state(100, 140, 100, 140, 30, &g_swipeTracker);

    // 2) If a swipe was just detected in that region, do something
    if (g_swipeTracker.swipeDetected) {
        switch(g_swipeTracker.swipeDir) {
        case SWIPE_DIR_LEFT:
            Serial.println("Left Swipe Action triggered!");
            // e.g. spin_stack(...) or do whatever you want
            break;
        case SWIPE_DIR_RIGHT:
            Serial.println("Right Swipe Action triggered!");
            break;
        case SWIPE_DIR_UP:
            Serial.println("Up Swipe Action triggered!");
            break;
        case SWIPE_DIR_DOWN:
            Serial.println("Down Swipe Action triggered!");
            break;
        default:
            // Shouldn't happen
            break;
        }
    }

    lv_timer_handler();
    delay(16);
}

