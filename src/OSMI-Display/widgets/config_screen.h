/**
 * @file config_screen.h
 * @author Jake Armstrong (armstrjj@rose-hulman.edu)
 * @brief Helper Function for config screen stuff.
 * @version 0.1
 * @date 2024-01-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef config_screen_h_
#define config_screen_h_
#include <lvgl.h>

typedef struct {
/**Static prototypes*/
    lv_obj_t *config_screen;

    osmi_roller_selector infusion_rate;
    osmi_roller_selector bolus_volume;
    osmi_roller_selector infusion_volume;

    lv_obj_t *confirm_button;
    lv_obj_t *cancel_button;

    int cs_pointer_callback;
} config_screen_t;

lv_obj_t* create_config_screen();
#endif