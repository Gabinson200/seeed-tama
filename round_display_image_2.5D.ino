#include <lvgl.h>
#define USE_ARDUINO_GFX_LIBRARY
#include "lv_xiao_round_screen.h"
#include "dino_sprites.h"

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

// We'll store up to 20 sprites
static pivot_sprite_t g_sprites[20];
static lv_anim_t      g_anim_storage[20];
static uint16_t       g_sprite_count = 0;

// A vertical pivot line in global coords
static const lv_coord_t g_pivot_line_x = 120;

// ---------------------------------------------------------
//  Animation callbacks
// ---------------------------------------------------------
static void set_angle(void * var, int32_t v) {
    pivot_sprite_t  * sq_data = (pivot_sprite_t  *)var;
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
    lv_img_set_pivot(obj, img_width/2, img_height/2 - pivot_offset_y); // Adjusted pivot

    // Position
    lv_coord_t x_pos = sq_data->base_x - (img_width/2);
    float layer_multiplier = 1.0 + (sprite_index * 1.2); // increasing change in y offset
    lv_coord_t y_pos = sq_data->base_y - (layer_multiplier * 2);
    lv_obj_set_pos(obj, x_pos, y_pos);

}

// Optional if you do zoom animations
static void set_zoom(void * img, int32_t v) {
    pivot_sprite_t  * sq_data = (pivot_sprite_t  *)img;
    lv_obj_t        * obj     = sq_data->obj;
    lv_img_set_zoom(obj, v);
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
        lv_coord_t x_pos = base_x; // Base x location provided by the user
        lv_coord_t y_pos = base_y - (i * spacing_y); // Adjust y position based on layer index

        lv_obj_set_pos(sprite_img, x_pos, y_pos);        

        // Store
        g_sprites[i].obj = sprite_img;
        g_sprites[i].base_x = base_x;
        g_sprites[i].base_y = base_y;

        // Rotation animation
        lv_anim_init(&g_anim_storage[i]);
        lv_anim_set_var(&g_anim_storage[i], &g_sprites[i]);
        lv_anim_set_exec_cb(&g_anim_storage[i], set_angle);
        lv_anim_set_time(&g_anim_storage[i], 10000);
        lv_anim_set_repeat_count(&g_anim_storage[i], LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_values(&g_anim_storage[i], 0, 3600);
        lv_anim_start(&g_anim_storage[i]);
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

    // 1) Initialize LVGL
    lv_init();

    // 2) Initialize the XIAO round display
    lv_xiao_disp_init();

    // 3) Create a bigger draw buffer
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, BUFF_SIZE);

    // 4) Get the default display
    lv_disp_t * disp = lv_disp_get_default();
    if(disp) {
        // In older LVGL, just do disp->driver:
        lv_disp_drv_t * disp_drv = (lv_disp_drv_t *)disp->driver;
        if(disp_drv) {
            // 5) Override the driver's draw_buf
            disp_drv->draw_buf = &draw_buf;
            disp_drv->hor_res = 240;
            disp_drv->ver_res = 240;
        }
    }

    // Create our rotating sprites
    create_sprites_pseudo_3d(/*num_sprites=*/15, /*base_x=*/120, /*base_y=*/120, /*spacing_y=*/1, /*zoom_step=*/0);
}

void loop() {
    lv_timer_handler();
    delay(16);
}
