#include "ui.h"
/*
// Global variables for the states
int hunger = 100;  // Hunger level (0 to 100)
int happiness = 100;  // Happiness level (0 to 100)
int energy = 100;  // Energy level (0 to 100)


// Function to update arcs based on state levels
void update_arcs(lv_obj_t *arc_hunger, lv_obj_t *arc_happiness, lv_obj_t *arc_energy) {
    lv_arc_set_value(arc_hunger, hunger);
    lv_arc_set_value(arc_happiness, happiness);
    lv_arc_set_value(arc_energy, energy);
}

// Timer callback to decrease states over time
static void timer_cb(lv_timer_t *timer) {
    hunger = (hunger > 0) ? hunger - 1 : 0;
    happiness = (happiness > 0) ? happiness - 1 : 0;
    energy = (energy > 0) ? energy - 1 : 0;

    // Update arcs
    lv_obj_t **arcs = (lv_obj_t **)timer->user_data;
    update_arcs(arcs[0], arcs[1], arcs[2]);
}

/*
void create_arcs(lv_obj_t *parent) {
    // Create arcs for hunger, happiness, and energy
    lv_obj_t *arc_hunger = lv_arc_create(parent);
    lv_obj_set_size(arc_hunger, 240, 240); // Full size for alignment
    lv_arc_set_range(arc_hunger, 0, 100);
    lv_arc_set_bg_angles(arc_hunger, 190, 230); // Arc spans 40 degrees
    lv_arc_set_value(arc_hunger, hunger);
    lv_obj_remove_style(arc_hunger, NULL, LV_PART_KNOB); // Remove center knob
    lv_obj_set_style_arc_color(arc_hunger, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red for Hunger
    lv_obj_set_style_arc_width(arc_hunger, 10, LV_PART_MAIN); // Arc thickness

    lv_obj_t *arc_happiness = lv_arc_create(parent);
    lv_obj_set_size(arc_happiness, 240, 240);
    lv_arc_set_range(arc_happiness, 0, 100);
    lv_arc_set_bg_angles(arc_happiness, 250, 290); // Arc spans 40 degrees
    lv_arc_set_value(arc_happiness, happiness);
    lv_obj_remove_style(arc_happiness, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_happiness, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(arc_happiness, lv_color_hex(0x00FF00), LV_PART_MAIN); // Red for Hunger
    lv_obj_set_style_arc_width(arc_happiness, 10, LV_PART_MAIN); // Arc thickness

    lv_obj_t *arc_energy = lv_arc_create(parent);
    lv_obj_set_size(arc_energy, 240, 240);
    lv_arc_set_range(arc_energy, 0, 100);
    lv_arc_set_bg_angles(arc_energy, 310, 350); // Arc spans 40 degrees
    lv_arc_set_value(arc_energy, energy);
    lv_obj_remove_style(arc_energy, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_energy, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(arc_energy, lv_color_hex(0x0000FF), LV_PART_MAIN); // Red for Hunger
    lv_obj_set_style_arc_width(arc_energy, 10, LV_PART_MAIN); // Arc thickness
    // Create a timer to update the states
    lv_obj_t *arcs[] = {arc_hunger, arc_happiness, arc_energy};
    lv_timer_create(timer_cb, 1000, arcs); // Update every second
}
*/

