#ifndef system_screen_h_
#define system_screen_h_
#include <lvgl.h>
#include "osmi_roller_selector.h"
#include <OSMI-Control/FluidDeliveryController.h>
#include "config_screen.h"
#include "status_screen.h"

typedef struct
{
    /**Static prototypes*/
    lv_obj_t *system_screen;
    lv_obj_t *status_screen;
    lv_obj_t *config_screen;
    
    ESP32PwmSpiDriver* Driver = NULL;

    lv_timer_t *timer;

    lv_obj_t *syssetEdit_button;
    lv_obj_t *dossetEdit_button;
    lv_obj_t *currentrate_text;


    lv_obj_t *startinfu_button;
    lv_obj_t *resetinfufast_button;
    lv_obj_t *resetinfuslow_button;
    lv_obj_t *forwardinfufast_button;
    lv_obj_t *forwardinfuslow_button;
    lv_obj_t *entertestmode_button;
    lv_obj_t *exittestmode_button;

} system_screen_t;



void create_system_screen(system_screen_t* screen_state);

#endif