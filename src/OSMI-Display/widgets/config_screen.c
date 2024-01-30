#include "config_screen.h"
#include "osmi_roller_selector.h"

static lv_obj_t config_screen;
static osmi_roller_selector bolus_rate;
static osmi_roller_selector infusion_rate;
static osmi_roller_selector bolus_volume;
static osmi_roller_selector infusion_volume;


static lv_obj_t* create_config_screen() {
    config_screen = lv_obj_create(NULL);
    
    
}