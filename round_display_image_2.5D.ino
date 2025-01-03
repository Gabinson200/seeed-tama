#define USE_ARDUINO_GFX_LIBRARY
#include <lvgl.h>
#include "lv_xiao_round_screen.h"
#include "dino_sprites.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"
#include "animations.h"

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


static swipe_tracker_t g_swipeTracker = {
    SWIPE_IDLE, false, SWIPE_DIR_NONE, 
    0,0, 0,0, 0,0  // initializing startX, startY, currentX, currentY, lastGoodX, lastGoodY
};

// We'll store up to 20 sprites
pivot_sprite_t g_sprites[20];
lv_anim_t      g_anim_storage[20];
uint16_t       g_sprite_count = 0;

static int32_t sprite_current_angle = 0; // Current rotation angle for dino

void set_background_color(lv_color_t color) {
    lv_obj_t *screen = lv_scr_act(); // Get the active screen
    lv_obj_set_style_bg_color(screen, color, 0); // Set the background color
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0); // Ensure the background is fully opaque
}

void set_gradient_background(lv_color_t color_start, lv_color_t color_end, lv_grad_dir_t grad_dir) {
    lv_obj_t *screen = lv_scr_act(); // Get the active screen
    lv_obj_set_style_bg_color(screen, color_start, 0); // Starting color
    lv_obj_set_style_bg_grad_color(screen, color_end, 0); // Ending color
    lv_obj_set_style_bg_grad_dir(screen, grad_dir, 0); // Gradient direction
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0); // Ensure full opacity
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

    //background init
    lv_obj_remove_style_all(lv_scr_act()); // Reset styles
    //set_background_color(lv_color_hex(0x2ad430)); // Set background to uniform color
    set_gradient_background(lv_color_hex(0x3b9c1e), lv_color_hex(0x3fc716), LV_GRAD_DIR_VER); // Vertical gradient from red to blue


    // Create our rotating sprites
    create_sprites_pseudo_3d(/*num_sprites=*/15, /*base_x=*/120, /*base_y=*/90, /*spacing_y=*/1, /*zoom_step=*/0);
    Serial.println("Created sprites");

    Serial.println("Setup complete");
}

// Pseudocode
void loop() {

  swipe_anim(
      100, 140, 100, 140, 20, &g_swipeTracker, &sprite_current_angle,
      spin_stack,  // Left animation
      spin_stack,  // Right animation
      spin_stack,  // Up animation
      spin_stack,  // Down animation
      -200,        // Left offset
      200,         // Right offset
      0,         // Up offset
      0,        // Down offset
      600,         // Duration
      false        // Infinite
  );

  lv_timer_handler();
  delay(16);
}

