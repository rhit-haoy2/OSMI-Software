/**
 * @file config_screen.cpp
 * @author Jake Armstrong
 * @brief Helper function for creating the OSMI control config screen.
 * @version 0.1
 * @date 2024-01-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "config_screen.h"
#include "osmi_roller_selector.h"



static void confirm_button_handler(lv_event_t *event) {

}

void create_config_screen(config_screen_t* screen)
{
    lv_obj_t *temporary_label;
    config_screen = lv_obj_create(NULL);
    // configure screen layout.
    lv_obj_set_layout(config_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(config_screen, LV_FLEX_FLOW_COLUMN);

    const char *numeric_options = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";
    const char *default_rates_units = "ml/sec\nml/min\nml/h";

    // create unit selectors.
    //  For labels between objects.
    lv_obj_t *label = lv_label_create(config_screen);
    lv_label_set_text(label, "Bolus Rate:");
    osmi_roller_selector_create(config_screen, &bolus_rate, numeric_options, default_rates_units);

    label = lv_label_create(config_screen);
    lv_label_set_text(label, "Bolus Volume:");
    osmi_roller_selector_create(config_screen, &bolus_volume, numeric_options, "ml");

    label = lv_label_create(config_screen);
    lv_label_set_text(label, "Infusion Rate:");
    osmi_roller_selector_create(config_screen, &infusion_rate, numeric_options, default_rates_units);

    label = lv_label_create(config_screen);
    lv_label_set_text(label, "Infusion Volume:");
    osmi_roller_selector_create(config_screen, &infusion_volume, numeric_options, "ml");

    // confirm setttings button.
    confirm_button = lv_btn_create(config_screen);
    temporary_label = lv_label_create(confirm_button);
    lv_label_set_text(temporary_label, "Confirm");
    lv_obj_center(temporary_label);

    // cancel settings change button.
    static lv_style_t cancel_style;
    lv_style_init(&cancel_style);

    lv_style_set_bg_color(&cancel_style, lv_palette_main(LV_PALETTE_RED));

    cancel_button = lv_btn_create(config_screen);
    // lv_obj_remove_style_all(cancel_button);
    lv_obj_add_style(cancel_button, &cancel_style, 0);
    temporary_label = lv_label_create(cancel_button);
    lv_label_set_text(temporary_label, "Cancel");
    lv_obj_center(temporary_label);

    return config_screen;
}