#define USE_ARDUINO_GFX_LIBRARY
#include <lvgl.h>
#include "lv_xiao_round_screen.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"
#include "animations.h"
//include "ui.h"

//sprite hex code files
#include "dino_sprites.h"
#include "pizza.h"
#include "burger.h"
#include "bed.h"

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

LV_IMG_DECLARE(pizza000);
LV_IMG_DECLARE(pizza001);
LV_IMG_DECLARE(pizza002);
LV_IMG_DECLARE(pizza003);

LV_IMG_DECLARE(burger000);
LV_IMG_DECLARE(burger001);
LV_IMG_DECLARE(burger002);
LV_IMG_DECLARE(burger003);
LV_IMG_DECLARE(burger004);
LV_IMG_DECLARE(burger005);
LV_IMG_DECLARE(burger006);
LV_IMG_DECLARE(burger007);

LV_IMG_DECLARE(bed000);
LV_IMG_DECLARE(bed001);
LV_IMG_DECLARE(bed002);
LV_IMG_DECLARE(bed003);
LV_IMG_DECLARE(bed004);
LV_IMG_DECLARE(bed005);
LV_IMG_DECLARE(bed006);
LV_IMG_DECLARE(bed007);

static const lv_img_dsc_t *sprite_images[] = {
    &tile000, &tile001, &tile002, &tile003, &tile004,
    &tile005, &tile006, &tile007, &tile008, &tile009,
    &tile010, &tile011, &tile012, &tile013, &tile014
};

static const lv_img_dsc_t *pizza_images[] = {
    &pizza000, &pizza001, &pizza002, &pizza003
};

static const lv_img_dsc_t *burger_images[] = {
  &burger000, &burger001, &burger002, &burger003, &burger004,
  &burger005, &burger006, &burger007
};

static const lv_img_dsc_t *bed_images[] = {
  &bed000, &bed001, &bed002, &bed003, &bed004,
  &bed005, &bed006, &bed007
};

static swipe_tracker_t g_swipeTracker = {
    SWIPE_IDLE, false, SWIPE_DIR_NONE, 
    0,0, 0,0, 0,0  // initializing startX, startY, currentX, currentY, lastGoodX, lastGoodY
};

// We'll store Dino sprites here
static pivot_sprite_t g_sprites_dino[15];
static uint16_t       g_sprites_dino_count = 0;

// We'll store Pizza sprites here
static pivot_sprite_t g_sprites_pizza[4];
static uint16_t       g_sprites_pizza_count = 0;

// We'll store Pizza sprites here
static pivot_sprite_t g_sprites_burger[8];
static uint16_t       g_sprites_burger_count = 0;

// We'll store bed sprites here
static pivot_sprite_t g_sprites_bed[8];
static uint16_t       g_sprites_bed_count = 0;

static int32_t sprite_current_angle = 0; // Current rotation angle for dino
static int32_t pizza_current_angle = 0; // Current rotation angle for pizza
static int32_t burger_current_angle = 0; // Current rotation angle for burger
static int32_t bed_current_angle = 0; // Current rotation angle for bed

// Global variables for status values
uint8_t hunger_value = 100;    // Example initial values
uint8_t happiness_value = 50;
uint8_t energy_value = 100;

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

// Timer callback function to update values and arcs
static void timer_cb(lv_timer_t *timer) {
    // Decrease values
    hunger_value = (hunger_value >= 2) ? hunger_value - 2 : 0;
    happiness_value = (happiness_value >= 1) ? happiness_value - 1 : 0;  // Shared for both arcs
    energy_value = (energy_value >= 2) ? energy_value - 2 : 0;

    // Get the arc objects from the timer's user data
    lv_obj_t **arcs = (lv_obj_t **)timer->user_data;

    // Update hunger arc (fills clockwise)
    lv_arc_set_value(arcs[0], hunger_value);

    // Update left part of the middle arc
    lv_arc_set_value(arcs[1], happiness_value > 50 ? 50 : happiness_value);

    // Update right part of the middle arc
    lv_arc_set_value(arcs[2], happiness_value > 50 ? 50 : happiness_value);

    // Update energy arc (fills clockwise)
    lv_arc_set_value(arcs[3], energy_value);
}



// Arc creation function
void create_arcs(lv_obj_t *parent) {
    // Define common properties for all arcs
    const int ARC_SIZE = 230;          // Size of arcs (matches screen)
    const int ARC_WIDTH = 12;          // Width of arc lines
    const int ARC_RANGE = 40;          // Angular range of each arc in degrees
    const int ARC_SPACING = 40;        // Angular spacing between arcs in degrees
    const int offset = 8; 

    // Create hunger arc (left) - fills clockwise
    lv_obj_t *arc_hunger = lv_arc_create(parent);
    lv_obj_set_size(arc_hunger, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_hunger);
    lv_arc_set_rotation(arc_hunger, 270 - ARC_SPACING - 12);
    lv_arc_set_mode(arc_hunger, LV_ARC_MODE_NORMAL);
    lv_arc_set_bg_angles(arc_hunger, 0, ARC_RANGE/2);  // Set arc bounds
    lv_arc_set_range(arc_hunger, 0, 100);  // Value range (0-100)
    lv_arc_set_value(arc_hunger, hunger_value);  // Initial value
    lv_obj_set_style_arc_width(arc_hunger, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_hunger, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_hunger, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_hunger, lv_color_hex(0xFF5555), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_hunger, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_hunger, LV_OBJ_FLAG_CLICKABLE);

    // Create happiness arc (middle-top) - fills symmetrically
    lv_obj_t *arc_happiness_left = lv_arc_create(parent);
    lv_obj_set_size(arc_happiness_left, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_happiness_left);
    lv_arc_set_rotation(arc_happiness_left, 260);
    lv_arc_set_mode(arc_happiness_left, LV_ARC_MODE_REVERSE);  // Symmetrical fill
    lv_arc_set_bg_angles(arc_happiness_left, 0, ARC_RANGE / 4);  // Symmetrical bounds
    lv_arc_set_range(arc_happiness_left, 0, 50);  // Value range (0-100)
    lv_arc_set_value(arc_happiness_left, happiness_value > 50 ? 50 : happiness_value);
    lv_obj_set_style_arc_width(arc_happiness_left, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_happiness_left, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_happiness_left, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_happiness_left, lv_color_hex(0x55FF55), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_happiness_left, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_happiness_left, LV_OBJ_FLAG_CLICKABLE);

    // Create happiness arc (middle-top) - fills symmetrically
    lv_obj_t *arc_happiness_right = lv_arc_create(parent);
    lv_obj_set_size(arc_happiness_right, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_happiness_right);
    lv_arc_set_rotation(arc_happiness_right, 280-offset);
    lv_arc_set_mode(arc_happiness_right, LV_ARC_MODE_NORMAL);  // Symmetrical fill
    lv_arc_set_bg_angles(arc_happiness_right, 0, ARC_RANGE / 4);  // Symmetrical bounds
    lv_arc_set_range(arc_happiness_right, 0, 50);  // Value range (0-100)
    lv_arc_set_value(arc_happiness_right, happiness_value > 50 ? 50 : happiness_value);
    lv_obj_set_style_arc_width(arc_happiness_right, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_happiness_right, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_happiness_right, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_happiness_right, lv_color_hex(0x55FF55), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_happiness_right, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_happiness_right, LV_OBJ_FLAG_CLICKABLE);


    // Create energy arc (right) - fills counter-clockwise
    lv_obj_t *arc_energy = lv_arc_create(parent);
    lv_obj_set_size(arc_energy, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_energy);
    lv_arc_set_rotation(arc_energy, 270 + ARC_SPACING - offset);
    lv_arc_set_mode(arc_energy, LV_ARC_MODE_REVERSE);  // Normal mode
    lv_arc_set_bg_angles(arc_energy, 0, ARC_RANGE/2);  // Clockwise bounds
    lv_arc_set_range(arc_energy, 0, 100);  // Value range (0-100)
    lv_arc_set_value(arc_energy, energy_value);  // Initial value
    lv_obj_set_style_arc_width(arc_energy, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_energy, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_energy, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_energy, lv_color_hex(0x5555FF), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_energy, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_energy, LV_OBJ_FLAG_CLICKABLE);

    // Store arcs in an array for the timer callback
    static lv_obj_t *arcs[4] = {arc_hunger, arc_happiness_left, arc_happiness_right, arc_energy};

    // Create a timer to update values and arcs every second
    lv_timer_create(timer_cb, 1000, arcs);
}



// ---------------------------------------------------------
//  Create sprites with pseudo-3D effect
// ---------------------------------------------------------
void create_sprites_pseudo_3d(const lv_img_dsc_t *image_list[],
                              pivot_sprite_t      *sprite_array,
                              uint16_t            num_sprites,
                              lv_coord_t          base_x,
                              lv_coord_t          base_y,
                              lv_coord_t          spacing_y,
                              uint16_t            zoom_step)
{
    uint16_t base_zoom = 256; // default 1:1 scale

    for (uint16_t i = 0; i < num_sprites; i++) {
        lv_obj_t *sprite_img = lv_img_create(lv_scr_act());
        if (!sprite_img) {
            Serial.println("Failed to create image object");
            continue;
        }

        // Set the image source
        const lv_img_dsc_t *src = image_list[i % num_sprites];
        lv_img_set_src(sprite_img, src);

        // Pseudo-3D layering by adjusting zoom
        uint16_t current_zoom = base_zoom + (i * zoom_step);
        lv_img_set_zoom(sprite_img, current_zoom);

        // Position
        lv_coord_t sprite_width = src->header.w;
        lv_coord_t x_pos = base_x - (sprite_width / 2);
        lv_coord_t y_pos = base_y - (i * spacing_y);
        lv_obj_set_pos(sprite_img, x_pos, y_pos);

        // Store into the chosen sprite_array
        sprite_array[i].obj    = sprite_img;
        sprite_array[i].base_x = base_x;
        sprite_array[i].base_y = base_y;
        sprite_array[i].index   = i;
        sprite_array[i].total_sprites = num_sprites;

        // Initialize angle
        lv_img_set_angle(sprite_img, 0);
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

    //arcs
    // Create a screen
    //lv_obj_t *screen = lv_scr_act();
    //create_arcs(screen);

    // Load the screen
    //lv_scr_load(screen);

    // Create our rotating sprites
        // For Dino sprites
    g_sprites_dino_count = 15;  // e.g. 15 dino frames
    create_sprites_pseudo_3d(
        sprite_images,        // e.g. your dino set
        g_sprites_dino,
        g_sprites_dino_count,
        /*base_x=*/120,
        /*base_y=*/90,
        /*spacing_y=*/1,
        /*zoom_step=*/0
    );

    // For Pizza sprites
    g_sprites_pizza_count = 4; // e.g. 4 pizza frames
    create_sprites_pseudo_3d(
        pizza_images,         // e.g. your pizza set
        g_sprites_pizza,
        g_sprites_pizza_count,
        /*base_x=*/60,
        /*base_y=*/170,
        /*spacing_y=*/1,
        /*zoom_step=*/0
    );

    // For Burger sprites
    g_sprites_burger_count = 8; // e.g. 4 pizza frames
    create_sprites_pseudo_3d(
        burger_images,         // e.g. your pizza set
        g_sprites_burger,
        g_sprites_burger_count,
        /*base_x=*/180,
        /*base_y=*/170,
        /*spacing_y=*/1,
        /*zoom_step=*/0
    );

        // For Burger sprites
    g_sprites_bed_count = 8; // e.g. 4 pizza frames
    create_sprites_pseudo_3d(
        bed_images,         // e.g. your pizza set
        g_sprites_bed,
        g_sprites_bed_count,
        /*base_x=*/120,
        /*base_y=*/170,
        /*spacing_y=*/1,
        /*zoom_step=*/0
    );

    // Create a screen
    lv_obj_t *screen = lv_scr_act();
    create_arcs(screen);

    // Load the screen
    lv_scr_load(screen);

    Serial.println("Created sprites");

    Serial.println("Setup complete");
}



// Pseudocode
void loop() {


  swipe_anim(
        100, 140,    // x_min, x_max
        100, 140,    // y_min, y_max
        20,          // min_swipe_length
        &g_swipeTracker,
        &sprite_current_angle,

        // Pass the DINO array + count:
        g_sprites_dino,
        g_sprites_dino_count,

        // The 4 animation callbacks
        stack_anim_pet,
        stack_anim_pet,
        stack_anim_pet,
        stack_anim_pet,

        // Offsets for left/right/up/down
         200,  // left_offset
         -200,  // right_offset
           0,  // up_offset
           0,  // down_offset

        // Duration, infinite
        600,
        false
    );

  touch_anim(
      50, 70,      // x_min, x_max
      160, 180,      // y_min, y_max
      g_sprites_pizza,
      g_sprites_pizza_count,
      &pizza_current_angle,
      stack_anim_item,    // the animation function
      3600,           // end_angle_offset
      1600,           // duration ms
      false          // infinite?
  );

    touch_anim(
      170, 190,      // x_min, x_max
      160, 180,      // y_min, y_max
      g_sprites_burger,
      g_sprites_burger_count,
      &burger_current_angle,
      stack_anim_item,    // the animation function
      3600,           // end_angle_offset
      1600,           // duration ms
      false          // infinite?
  );

      touch_anim(
      100, 140,      // x_min, x_max
      160, 180,      // y_min, y_max
      g_sprites_bed,
      g_sprites_bed_count,
      &bed_current_angle,
      stack_anim_rotate,    // the animation function
      3600,           // end_angle_offset
      2000,           // duration ms
      false          // infinite?
  );

  lv_timer_handler();
  delay(16);
}

