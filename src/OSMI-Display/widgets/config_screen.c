#include "config_screen.h"
#include "osmi_roller_selector.h"

/**Static prototypes*/
static lv_obj_t *config_screen;

static osmi_roller_selector bolus_rate;
static osmi_roller_selector infusion_rate;
static osmi_roller_selector bolus_volume;
static osmi_roller_selector infusion_volume;

static lv_obj_t *confirm_button;
static lv_obj_t *cancel_button;

static int cs_pointer_callback;

lv_obj_t *create_config_screen()
{
    lv_obj_t* temporary_label;
    config_screen = lv_obj_create(NULL);
    // configure screen layout.
    lv_obj_set_layout(config_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(config_screen, LV_FLEX_FLOW_COLUMN);

    // create unit selectors.
    osmi_roller_selector_create(config_screen, &bolus_rate, numeric_options, default_rates_units);
    osmi_roller_selector_create(config_screen, &bolus_volume, numeric_options, "ml\n");
    osmi_roller_selector_create(config_screen, &infusion_rate, numeric_options, default_rates_units);
    osmi_roller_selector_create(config_screen, &infusion_volume, numeric_options, "ml\n");

    // confirm setttings button.

    confirm_button = lv_btn_create(config_screen);
    temporary_label = lv_label_create(confirm_button);
    lv_label_set_text(temporary_label, "Confirm");
    lv_obj_center(temporary_label);


    cancel_button = lv_btn_create(config_screen);
    temporary_label = lv_label_create(cancel_button);
    lv_label_set_text(temporary_label, "Cancel");
    lv_obj_center(temporary_label);


    return config_screen;
}