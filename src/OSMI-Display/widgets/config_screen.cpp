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

static void confirm_button_handler(lv_event_t *event)
{
    config_screen_t *screen = (config_screen_t *)lv_obj_get_user_data(event->target);
    // todo make sure values make sense.

    screen->controller->stopFlow();
    screen->controller->configureDosage(screen->bolus_rate.value, screen->bolus_volume.value, screen->infusion_rate.value, screen->infusion_volume.value);
    screen->controller->startFlow();
    //todo add modal to show status.
    Serial.println("Start Button Pressed.");
}
static void cancel_button_handler(lv_event_t *event)
{
    config_screen_t *screen = (config_screen_t *)lv_obj_get_user_data(event->target);
    // currently a makeshift stop button.
    screen->controller->stopFlow();
    Serial.println("Stop Button Pressed.");
}

void create_config_screen(config_screen_t *screen)
{
    lv_obj_t *temporary_label;
    screen->config_screen = lv_obj_create(NULL);
    // configure screen layout.
    lv_obj_set_layout(screen->config_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen->config_screen, LV_FLEX_FLOW_COLUMN);

    const char *numeric_options = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";
    const char *default_rates_units = "ml/sec\nml/min\nml/h";

    // create unit selectors.
    //  For labels between objects.
    lv_obj_t *label = lv_label_create(screen->config_screen);
    lv_label_set_text(label, "Bolus Rate:");
    osmi_roller_selector_create(screen->config_screen, &screen->bolus_rate, numeric_options, default_rates_units);

    label = lv_label_create(screen->config_screen);
    lv_label_set_text(label, "Bolus Volume:");
    osmi_roller_selector_create(screen->config_screen, &screen->bolus_volume, numeric_options, "ml");

    label = lv_label_create(screen->config_screen);
    lv_label_set_text(label, "Infusion Rate:");
    osmi_roller_selector_create(screen->config_screen, &screen->infusion_rate, numeric_options, default_rates_units);

    label = lv_label_create(screen->config_screen);
    lv_label_set_text(label, "Infusion Volume:");
    osmi_roller_selector_create(screen->config_screen, &screen->infusion_volume, numeric_options, "ml");

    // confirm setttings button.
    screen->confirm_button = lv_btn_create(screen->config_screen);
    temporary_label = lv_label_create(screen->confirm_button);
    lv_label_set_text(temporary_label, "Confirm");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->confirm_button, screen);
    lv_obj_add_event_cb(screen->confirm_button, confirm_button_handler, LV_EVENT_RELEASED, screen);

    // cancel settings change button.
    static lv_style_t cancel_style;
    lv_style_init(&cancel_style);

    lv_style_set_bg_color(&cancel_style, lv_palette_main(LV_PALETTE_RED));

    screen->cancel_button = lv_btn_create(screen->config_screen);
    // lv_obj_remove_style_all(cancel_button);
    lv_obj_add_style(screen->cancel_button, &cancel_style, 0);
    temporary_label = lv_label_create(screen->cancel_button);
    lv_label_set_text(temporary_label, "Cancel");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->cancel_button, screen);
    lv_obj_add_event_cb(screen->cancel_button, cancel_button_handler, LV_EVENT_RELEASED, screen);
}

float config_screen_get_bolus_rate(config_screen_t *screen)
{
    return screen->bolus_rate.value;
}
float config_screen_get_bolus_volume(config_screen_t *screen)
{
    return screen->bolus_volume.value;
}
float config_screen_get_infusion_rate(config_screen_t *screen)
{
    return screen->infusion_rate.value;
}
float config_screen_get_infusion_volume(config_screen_t *screen)
{
    return screen->infusion_volume.value;
}
