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
#include "osmi_roller_selector.h"
#include <OSMI-Control/FluidDeliveryController.h>

typedef struct
{
    /**Static prototypes*/
    lv_obj_t *config_screen;
    
    Team11Control* controller = NULL;

    osmi_roller_selector bolus_rate;
    osmi_roller_selector infusion_rate;
    osmi_roller_selector bolus_volume;
    osmi_roller_selector infusion_volume;

    lv_obj_t *confirm_button;
    lv_obj_t *cancel_button;

    lv_obj_t *status_screen;
    lv_obj_t *system_screen;

} config_screen_t;

/// @brief Create the screen config.
/// @param  screen_state A `config_screen_t` to hold the screen's state.
/// @return nothing.
void create_config_screen(config_screen_t* screen_state);

float config_screen_get_bolus_rate(config_screen_t* screen);
float config_screen_get_bolus_volume(config_screen_t* screen);
float config_screen_get_infusion_rate(config_screen_t* screen);
float config_screen_get_infusion_volume(config_screen_t* screen);
#endif