#ifndef status_screen_h_
#define status_screen_h_
#include <lvgl.h>
#include "osmi_roller_selector.h"
#include <OSMI-Control/FluidDeliveryController.h>
#include "config_screen.h"

typedef struct
{
    /**Static prototypes*/
    lv_obj_t *status_screen;
    config_screen_t *config_screen;
    
    Team11Control* controller = NULL;

    lv_timer_t *timer;

    lv_obj_t *currentrate_text;
    lv_obj_t *timeleft_text;
    lv_obj_t *bolus_bar;
    lv_obj_t *infusion_bar;


    lv_obj_t *pause_button;
} status_screen_t;



void create_status_screen(status_screen_t* screen_state);

#endif