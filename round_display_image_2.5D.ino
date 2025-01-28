#define USE_ARDUINO_GFX_LIBRARY
#include <lvgl.h>
#include "lv_xiao_round_screen.h"
#include "touch_swipe.h"
#include "touch_sensor_functions.h"
#include "animations.h"

// ------------------- Arduino & IMU includes -------------------
#include <Arduino.h>
#include <LSM6DS3.h>
#include <Wire.h>

// ------------------- TensorFlow Lite includes -------------------
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

// IMPORTANT: remove or comment out the old static model references if you like
// them to be in flash only. We'll load them in a function on-demand.
#include "model.h"  // your gesture_cnn_int8_model[] or dense int8 model

//###############################################################
//                IMU setup
//###############################################################

LSM6DS3 myIMU(I2C_MODE, 0x6A);

// For demonstration, we'll do a single small buffer read in doSingleInferenceOnce()
static const float accelerationThreshold = 2.3; // example threshold
static const int   numSamples            = 179;  // 1.5 seconds worth of data
int samplesRead = numSamples; // initially = done state

// If your model class names differ, adapt these:
static const char* GESTURES[] = {"bow", "sleep", "circle"};
static const int   NUM_GESTURES = 3;  // or however many classes you have

//###############################################################
//            UI vars and functions
//###############################################################

//sprite hex code files
#include "dino_sprites.h"
#include "pizza.h"
#include "burger.h"
#include "bed.h"

// Example image declarations
LV_IMG_DECLARE(tile000);  LV_IMG_DECLARE(tile001);  LV_IMG_DECLARE(tile002);
LV_IMG_DECLARE(tile003);  LV_IMG_DECLARE(tile004);  LV_IMG_DECLARE(tile005);
LV_IMG_DECLARE(tile006);  LV_IMG_DECLARE(tile007);  LV_IMG_DECLARE(tile008);
LV_IMG_DECLARE(tile009);  LV_IMG_DECLARE(tile010);  LV_IMG_DECLARE(tile011);
LV_IMG_DECLARE(tile012);  LV_IMG_DECLARE(tile013);  LV_IMG_DECLARE(tile014);

LV_IMG_DECLARE(pizza000); LV_IMG_DECLARE(pizza001); LV_IMG_DECLARE(pizza002);
LV_IMG_DECLARE(pizza003);

LV_IMG_DECLARE(burger000); LV_IMG_DECLARE(burger001); LV_IMG_DECLARE(burger002);
LV_IMG_DECLARE(burger003); LV_IMG_DECLARE(burger004); LV_IMG_DECLARE(burger005);
LV_IMG_DECLARE(burger006); LV_IMG_DECLARE(burger007);

LV_IMG_DECLARE(bed000);    LV_IMG_DECLARE(bed001);    LV_IMG_DECLARE(bed002);
LV_IMG_DECLARE(bed003);    LV_IMG_DECLARE(bed004);    LV_IMG_DECLARE(bed005);
LV_IMG_DECLARE(bed006);    LV_IMG_DECLARE(bed007);

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
    0,0, 0,0, 0,0
};

static pivot_sprite_t g_sprites_dino[15];
static uint16_t       g_sprites_dino_count = 0;

static pivot_sprite_t g_sprites_pizza[4];
static uint16_t       g_sprites_pizza_count = 0;

static pivot_sprite_t g_sprites_burger[8];
static uint16_t       g_sprites_burger_count = 0;

static pivot_sprite_t g_sprites_bed[8];
static uint16_t       g_sprites_bed_count = 0;

static int32_t sprite_current_angle = 0;
static int32_t pizza_current_angle  = 0;
static int32_t burger_current_angle = 0;
static int32_t bed_current_angle    = 0;

// Global variables for status arcs
uint8_t hunger_value    = 100;
uint8_t happiness_value = 50;
uint8_t energy_value    = 100;

// Utility to set background gradient
void set_gradient_background() {
    // Create top half (sky) gradient
    lv_obj_t *top_bg = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(top_bg);                 // Remove default styles
    lv_obj_set_size(top_bg, 240, 120);               // Width=240, Height=120
    lv_obj_align(top_bg, LV_ALIGN_TOP_LEFT, 0, 0);   // Place at (0,0)

    // Sky gradient: Light blue -> Darker blue, vertical
    lv_obj_set_style_bg_color(top_bg, lv_color_hex(0x4682B4), 0);  // Light blue
    lv_obj_set_style_bg_grad_color(top_bg, lv_color_hex(0x87CEEB), 0);  // Darker blue
    lv_obj_set_style_bg_grad_dir(top_bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(top_bg, LV_OPA_COVER, 0);

    // Create bottom half (grass) gradient
    lv_obj_t *bottom_bg = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(bottom_bg);
    lv_obj_set_size(bottom_bg, 240, 120);            // Another 240×120
    // Position it immediately below the top half (y=120)
    lv_obj_align(bottom_bg, LV_ALIGN_TOP_LEFT, 0, 120);

    // Grass gradient: Light green -> Darker green, vertical
    lv_obj_set_style_bg_color(bottom_bg, lv_color_hex(0x3CB371), 0);  // Light green
    lv_obj_set_style_bg_grad_color(bottom_bg, lv_color_hex(0x98FB98), 0); // Darker green
    lv_obj_set_style_bg_grad_dir(bottom_bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(bottom_bg, LV_OPA_COVER, 0);
}


// Timer callback function to update arcs
static void timer_cb(lv_timer_t *timer) {
    hunger_value    = (hunger_value >= 2)     ? hunger_value - 2 : 0;
    happiness_value = (happiness_value >= 1)  ? happiness_value - 1 : 0;
    energy_value    = (energy_value >= 2)     ? energy_value - 2 : 0;

    lv_obj_t **arcs = (lv_obj_t **)timer->user_data;
    // arcs[0] = hunger arc
    lv_arc_set_value(arcs[0], hunger_value);

    // arcs[1], arcs[2] = happiness arcs
    lv_arc_set_value(arcs[1], (happiness_value > 50 ? 50 : happiness_value));
    lv_arc_set_value(arcs[2], (happiness_value > 50 ? 50 : happiness_value));

    // arcs[3] = energy arc
    lv_arc_set_value(arcs[3], energy_value);
}

// Arc creation function
void create_arcs(lv_obj_t *parent) {
    const int ARC_SIZE   = 230;
    const int ARC_WIDTH  = 12;
    const int ARC_RANGE  = 40;
    const int ARC_SPACING= 40;
    const int offset     = 8; 

    // Hunger arc
    lv_obj_t *arc_hunger = lv_arc_create(parent);
    lv_obj_set_size(arc_hunger, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_hunger);
    lv_arc_set_rotation(arc_hunger, 270 - ARC_SPACING - 12);
    lv_arc_set_mode(arc_hunger, LV_ARC_MODE_NORMAL);
    lv_arc_set_bg_angles(arc_hunger, 0, ARC_RANGE/2);
    lv_arc_set_range(arc_hunger, 0, 100);
    lv_arc_set_value(arc_hunger, hunger_value);
    lv_obj_set_style_arc_width(arc_hunger, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_hunger, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_hunger, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_hunger, lv_color_hex(0xFF5555), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_hunger, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_hunger, LV_OBJ_FLAG_CLICKABLE);

    // Happiness arcs (two arcs forming symmetrical fill)
    lv_obj_t *arc_happiness_left = lv_arc_create(parent);
    lv_obj_set_size(arc_happiness_left, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_happiness_left);
    lv_arc_set_rotation(arc_happiness_left, 260);
    lv_arc_set_mode(arc_happiness_left, LV_ARC_MODE_REVERSE);
    lv_arc_set_bg_angles(arc_happiness_left, 0, ARC_RANGE / 4);
    lv_arc_set_range(arc_happiness_left, 0, 50);
    lv_arc_set_value(arc_happiness_left, (happiness_value > 50 ? 50 : happiness_value));
    lv_obj_set_style_arc_width(arc_happiness_left, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_happiness_left, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_happiness_left, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_happiness_left, lv_color_hex(0x55FF55), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_happiness_left, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_happiness_left, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *arc_happiness_right = lv_arc_create(parent);
    lv_obj_set_size(arc_happiness_right, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_happiness_right);
    lv_arc_set_rotation(arc_happiness_right, 280-offset);
    lv_arc_set_mode(arc_happiness_right, LV_ARC_MODE_NORMAL);
    lv_arc_set_bg_angles(arc_happiness_right, 0, ARC_RANGE / 4);
    lv_arc_set_range(arc_happiness_right, 0, 50);
    lv_arc_set_value(arc_happiness_right, (happiness_value > 50 ? 50 : happiness_value));
    lv_obj_set_style_arc_width(arc_happiness_right, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_happiness_right, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_happiness_right, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_happiness_right, lv_color_hex(0x55FF55), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_happiness_right, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_happiness_right, LV_OBJ_FLAG_CLICKABLE);

    // Energy arc
    lv_obj_t *arc_energy = lv_arc_create(parent);
    lv_obj_set_size(arc_energy, ARC_SIZE, ARC_SIZE);
    lv_obj_center(arc_energy);
    lv_arc_set_rotation(arc_energy, 270 + ARC_SPACING - offset);
    lv_arc_set_mode(arc_energy, LV_ARC_MODE_REVERSE);
    lv_arc_set_bg_angles(arc_energy, 0, ARC_RANGE/2);
    lv_arc_set_range(arc_energy, 0, 100);
    lv_arc_set_value(arc_energy, energy_value);
    lv_obj_set_style_arc_width(arc_energy, ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_energy, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_energy, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_energy, lv_color_hex(0x5555FF), LV_PART_INDICATOR);
    lv_obj_remove_style(arc_energy, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_energy, LV_OBJ_FLAG_CLICKABLE);

    // Store arcs for timer
    static lv_obj_t *arcs[4] = {arc_hunger, arc_happiness_left, arc_happiness_right, arc_energy};
    lv_timer_create(timer_cb, 1000, arcs);
}

// Helper to create pseudo-3D sprites
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
        const lv_img_dsc_t *src = image_list[i % num_sprites];
        lv_img_set_src(sprite_img, src);

        uint16_t current_zoom = base_zoom + (i * zoom_step);
        lv_img_set_zoom(sprite_img, current_zoom);

        lv_coord_t sprite_width = src->header.w;
        lv_coord_t x_pos = base_x - (sprite_width / 2);
        lv_coord_t y_pos = base_y - (i * spacing_y);
        lv_obj_set_pos(sprite_img, x_pos, y_pos);

        sprite_array[i].obj          = sprite_img;
        sprite_array[i].base_x       = base_x;
        sprite_array[i].base_y       = base_y;
        sprite_array[i].index        = i;
        sprite_array[i].total_sprites= num_sprites;
        lv_img_set_angle(sprite_img, 0);
    }
}

// ---------------------------------------------------------
//  On-demand TFLite gesture recognition (One Iteration)
// ---------------------------------------------------------

#include <stdlib.h> // For malloc and free

int freeMemory() {
    // Allocate a block of memory and immediately free it
    // The size of the block is arbitrary but should be large enough to trigger heap allocation
    const size_t blockSize = 1024; // 1 KB
    void* block = malloc(blockSize);

    if (block == nullptr) {
        // If malloc fails, there is no free memory left
        return 0;
    }

    free(block); // Free the allocated block

    // Measure the remaining free memory by allocating increasingly larger blocks
    size_t freeMem = 0;
    while (true) {
        block = malloc(freeMem + blockSize);
        if (block == nullptr) {
            break; // Stop when malloc fails
        }
        free(block);
        freeMem += blockSize;
    }

    return freeMem;
}


// We'll wrap the original "base code" logic in this function.
// This runs a *blocking* 1.5s capture (if motion is detected).
void runGestureCaptureAndInferenceOnce()
{
  Serial.println("=== Start on-demand TFLite + IMU capture ===");

  // 1) Set up TFLite (allocate statically or dynamically).
  Serial.println("Step 1: Creating MicroErrorReporter & MicroMutableOpResolver...");
  static tflite::MicroErrorReporter tflErrorReporter;
  static tflite::MicroMutableOpResolver<12> tflOpsResolver;

  // Only add the ops your model needs:
  tflOpsResolver.AddFullyConnected();
  tflOpsResolver.AddConv2D();     // if CNN
  tflOpsResolver.AddMaxPool2D();
  tflOpsResolver.AddSoftmax();
  tflOpsResolver.AddReshape();
  tflOpsResolver.AddRelu();
  tflOpsResolver.AddShape();
  tflOpsResolver.AddReshape();
  tflOpsResolver.AddStridedSlice();
  tflOpsResolver.AddPack();
  tflOpsResolver.AddExpandDims();
  tflOpsResolver.AddMean();
  Serial.println("Step 1 done.");

  // 2) The Model
  Serial.println("Step 2: Getting model pointer...");
  const tflite::Model* tflModel = tflite::GetModel(model);
  Serial.print("Model version: ");
  Serial.println(tflModel->version());
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch!");
    return;
  }
  Serial.println("Step 2 done.");

  // 3) Interpreter
  Serial.println("Step 3: Creating MicroInterpreter...");
  static constexpr int tensorArenaSize = 6 * 1024;
  static uint8_t tensorArena[tensorArenaSize] __attribute__((aligned(16)));

  tflite::MicroInterpreter* tflInterpreter =
      new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena,
                                   tensorArenaSize, &tflErrorReporter);
  if(!tflInterpreter) {
    Serial.println("Failed to create MicroInterpreter!");
    return;
  }
  Serial.println("Step 3: MicroInterpreter created successfully.");

  // 4) Allocate Tensors
  Serial.println("Step 4: Allocating Tensors...");
  TfLiteStatus allocStatus = tflInterpreter->AllocateTensors();
  Serial.println("AllocateTensors call returned...");
  if (allocStatus != kTfLiteOk) {
    Serial.println("AllocateTensors() failed!");
    delete tflInterpreter;
    return;
  }
  Serial.println("Tensors allocated successfully.");

  // 5) Print input shape (to confirm the model shape)
  TfLiteTensor* tflInputTensor  = tflInterpreter->input(0);
  TfLiteTensor* tflOutputTensor = tflInterpreter->output(0);
  Serial.print("Input dims: ");
  for (int i = 0; i < tflInputTensor->dims->size; i++) {
    Serial.print(tflInputTensor->dims->data[i]);
    Serial.print(" ");
  }
  Serial.println();

  // 4) Get input & output quantization parameters
  float input_scale       = 1.0f;
  int   input_zero_point  = 0;
  if (tflInputTensor->type == kTfLiteInt8) {
    auto* quant_params_in = (TfLiteAffineQuantization*) tflInputTensor->quantization.params;
    if (quant_params_in) {
      input_scale      = quant_params_in->scale->data[0];
      input_zero_point = quant_params_in->zero_point->data[0];
    }
  }
  float output_scale      = 1.0f;
  int   output_zero_point = 0;
  if (tflOutputTensor->type == kTfLiteInt8) {
    auto* quant_params_out = (TfLiteAffineQuantization*) tflOutputTensor->quantization.params;
    if (quant_params_out) {
      output_scale      = quant_params_out->scale->data[0];
      output_zero_point = quant_params_out->zero_point->data[0];
    }
  }

  // 5) Prepare to capture 179 samples for 1.5s window
  samplesRead = numSamples; // indicates we are "idle"
  unsigned long lastGestureStartTime = 0;

  // The logic from your base code:

  // Step A) Wait until motion is detected above threshold
  while (true) {
    float aX = myIMU.readFloatAccelX();
    float aY = myIMU.readFloatAccelY();
    float aZ = myIMU.readFloatAccelZ();

    float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

    // Debug print:
    Serial.print("aX=");
    Serial.print(aX, 2);
    Serial.print(" aY=");
    Serial.print(aY, 2);
    Serial.print(" aZ=");
    Serial.print(aZ, 2);
    Serial.print(" sum=");
    Serial.println(aSum, 2);

    if (aSum >= accelerationThreshold) {
      // Begin collecting
      samplesRead = 0;
      lastGestureStartTime = millis();
      break;
    }
    delay(100); // short delay to avoid busy loop
  }

  // Step B) Collect IMU data until we have numSamples
  while (samplesRead < numSamples) {
    float aX = myIMU.readFloatAccelX();
    float aY = myIMU.readFloatAccelY();
    float aZ = myIMU.readFloatAccelZ();
    float gX = myIMU.readFloatGyroX();
    float gY = myIMU.readFloatGyroY();
    float gZ = myIMU.readFloatGyroZ();

    // Normalize, quantize, store
    int sample_index = samplesRead * 6;

    float ax_norm = (aX + 4.0f) / 8.0f;
    float ay_norm = (aY + 4.0f) / 8.0f;
    float az_norm = (aZ + 4.0f) / 8.0f;
    float gx_norm = (gX + 2000.0f) / 4000.0f;
    float gy_norm = (gY + 2000.0f) / 4000.0f;
    float gz_norm = (gZ + 2000.0f) / 4000.0f;

    tflInputTensor->data.int8[sample_index + 0] = (int8_t)roundf((ax_norm / input_scale) + input_zero_point);
    tflInputTensor->data.int8[sample_index + 1] = (int8_t)roundf((ay_norm / input_scale) + input_zero_point);
    tflInputTensor->data.int8[sample_index + 2] = (int8_t)roundf((az_norm / input_scale) + input_zero_point);
    tflInputTensor->data.int8[sample_index + 3] = (int8_t)roundf((gx_norm / input_scale) + input_zero_point);
    tflInputTensor->data.int8[sample_index + 4] = (int8_t)roundf((gy_norm / input_scale) + input_zero_point);
    tflInputTensor->data.int8[sample_index + 5] = (int8_t)roundf((gz_norm / input_scale) + input_zero_point);

    samplesRead++;

    // If done collecting, run inference
    if (samplesRead >= numSamples) {
      // Perform inference
      TfLiteStatus invokeStatus = tflInterpreter->Invoke();
      if (invokeStatus != kTfLiteOk) {
        Serial.println("Invoke failed!");
        break;
      }

      // Print output predictions
      for (int i = 0; i < NUM_GESTURES; i++) {
        int8_t outVal = tflOutputTensor->data.int8[i];
        float prob = (outVal - output_zero_point) * output_scale;
        Serial.print(GESTURES[i]);
        Serial.print(": ");
        Serial.println(prob, 3);
      }
      Serial.println();
    }

    delay(5);
  }

  // 6) Cleanup interpreter
  delete tflInterpreter;
  Serial.println("=== Done capturing + inference ===");
}

static void bed_animation_complete_cb(lv_anim_t * anim)
{
    Serial.print("Free RAM before TFLM: ");
    Serial.println(freeMemory());

    // Now invoke TensorFlow inference
    runGestureCaptureAndInferenceOnce();
}

// ---------------------------------------------------------
//  Bed press callback - run rotation + single inference
// ---------------------------------------------------------
void bed_press_callback(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite)
{
    stack_anim_rotate_with_inference(
        sprites,
        sprite_count,
        current_angle,
        end_angle_offset,
        duration,
        infinite
    );
}


static void stack_anim_rotate_with_inference(
    pivot_sprite_t *sprites,
    uint16_t        sprite_count,
    int32_t        *current_angle,
    int32_t         end_angle_offset,
    uint32_t        duration,
    bool            infinite)
{
    // 1) Make the local array 'static' so it persists after this function returns
    lv_anim_t local_anim_storage[sprite_count];

    int32_t start_angle = *current_angle;
    int32_t end_angle   = start_angle + end_angle_offset;

    // 2) Loop over each sprite in the stack
    for (uint16_t i = 0; i < sprite_count && i < sprite_count; i++) {

        // Stop any ongoing animations on this sprite
        lv_anim_del(sprites[i].obj, NULL);

        // Initialize the animation
        lv_anim_init(&local_anim_storage[i]);

        // Set all parameters BEFORE lv_anim_start
        lv_anim_set_var(&local_anim_storage[i], &sprites[i]);
        lv_anim_set_exec_cb(&local_anim_storage[i], rotate_anim);
        lv_anim_set_time(&local_anim_storage[i], duration);
        lv_anim_set_values(&local_anim_storage[i], start_angle, end_angle);

        // If we want an infinite loop, no finish callback is possible
        if(infinite) {
            lv_anim_set_repeat_count(&local_anim_storage[i], LV_ANIM_REPEAT_INFINITE);
        } 
        else {
            // Attach the finish callback ONLY on the last sprite in the stack
            // so that it runs once the entire stack is finished
            if(i == sprite_count - 1) {
                lv_anim_set_ready_cb(&local_anim_storage[i], bed_animation_complete_cb);
            }
        }

        // Now start the animation
        lv_anim_start(&local_anim_storage[i]);
    }

    // 3) Update the "current_angle" so next time we know our new final
    *current_angle = end_angle;
}

// ---------------------------------------------------------
//  Arduino Setup
// ---------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(100);

    // IMU init (always on, or you could also re-init inside doSingleInferenceOnce())
    if(myIMU.begin() != 0) {
        Serial.println("IMU not found!");
    } else {
        Serial.println("IMU found!");
    }

    // LVGL Setup
    lv_init();
    lv_xiao_disp_init();
    lv_xiao_touch_init();

    set_gradient_background();

    // Create arcs on screen
    lv_obj_t *screen = lv_scr_act();
    create_arcs(screen);
    lv_scr_load(screen);

    // Create Dino sprites
    g_sprites_dino_count = 15;
    create_sprites_pseudo_3d(
        sprite_images, g_sprites_dino, g_sprites_dino_count,
        /*base_x=*/120, /*base_y=*/110, /*spacing_y=*/1, /*zoom_step=*/0
    );

    // Create Pizza sprites
    g_sprites_pizza_count = 4;
    create_sprites_pseudo_3d(
        pizza_images, g_sprites_pizza, g_sprites_pizza_count,
        /*base_x=*/60, /*base_y=*/180, /*spacing_y=*/1, /*zoom_step=*/0
    );

    // Create Burger sprites
    g_sprites_burger_count = 8;
    create_sprites_pseudo_3d(
        burger_images, g_sprites_burger, g_sprites_burger_count,
        /*base_x=*/180, /*base_y=*/180, /*spacing_y=*/1, /*zoom_step=*/0
    );

    // Create Bed sprites
    g_sprites_bed_count = 8;
    create_sprites_pseudo_3d(
        bed_images, g_sprites_bed, g_sprites_bed_count,
        /*base_x=*/120, /*base_y=*/190, /*spacing_y=*/1, /*zoom_step=*/0
    );

    Serial.println("Setup complete");
}

// ---------------------------------------------------------
//  Arduino Loop
// ---------------------------------------------------------
void loop() {

  // Existing swipe animation for Dino
  swipe_anim(
      100, 140,  // x_min, x_max
      100, 140,  // y_min, y_max
      20,        // min_swipe_length
      &g_swipeTracker,
      &sprite_current_angle,
      g_sprites_dino,
      g_sprites_dino_count,
      stack_anim_pet, stack_anim_pet, stack_anim_pet, stack_anim_pet,
      /*left_offset=*/200, /*right_offset=*/-200,
      /*up_offset=*/0,     /*down_offset=*/0,
      /*duration=*/600,
      false
  );

  // Pizza
  touch_anim(
      50, 70,      // x_min, x_max
      170, 190,    // y_min, y_max
      g_sprites_pizza,
      g_sprites_pizza_count,
      &pizza_current_angle,
      stack_anim_item, // animation function
      3600,            // end_angle_offset
      1600,            // duration
      false
  );

  // Burger
  touch_anim(
      170, 190,
      170, 190,
      g_sprites_burger,
      g_sprites_burger_count,
      &burger_current_angle,
      stack_anim_item,
      3600,
      1600,
      false
  );

  // ** Bed sprite **
  // Replace stack_anim_rotate with our bed_press_callback
  touch_anim(
      100, 140,
      180, 200,
      g_sprites_bed,
      g_sprites_bed_count,
      &bed_current_angle,
      bed_press_callback,  // <--- calls single inference inside
      3600,
      2000,
      false
  );

  lv_timer_handler();
  delay(16);
}
