#include "osmi_roller_selector.h"
#include <stdio.h>


/**
 * @brief Handles when any of the values change.
 *
 * @param event LVGL Event object.
 */
static void osmi_roller_event_handler(lv_event_t *event)
{
    osmi_roller_selector *this = (osmi_roller_selector *)lv_event_get_user_data(event);
    if (this == NULL)
        return;
    printf("DEBUG: this* %p", this);
    printf("DEBUG: this->roller_tens: %p", this->roller_tens);
    float tens = lv_roller_get_selected(this->roller_tens);
    float ones = lv_roller_get_selected(this->roller_ones);
    float tenths = lv_roller_get_selected(this->roller_tenths);

    this->value = (tens * 10) + ones + (tenths * 0.1);
    // TODO Handle different units.
}

float osmi_roller_get_value(osmi_roller_selector *this)
{
    return this->value;
}

void osmi_roller_selector_create(lv_obj_t *parent, osmi_roller_selector *this, const char* numeric_options, char* units)
{
    this->value = 0.0;

    this->container = lv_obj_create(parent);
    lv_obj_set_size(this->container, 200, 100);
    // Container flex allign settings.
    lv_obj_set_flex_flow(this->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(this->container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_scrollbar_mode(this->container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(this->container, LV_OBJ_FLAG_SCROLLABLE);

    this->roller_tens = lv_roller_create(this->container);
    this->roller_ones = lv_roller_create(this->container);
    this->roller_tenths = lv_roller_create(this->container);
    this->roller_units = lv_roller_create(this->container);

    // Units roller settings.
    lv_roller_set_options(this->roller_units, , LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(this->roller_units, 2);
    lv_obj_add_flag(this->roller_ones, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    // Numeric value settings.
    lv_roller_set_options(this->roller_ones, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(this->roller_ones, 2);
    lv_obj_add_flag(this->roller_ones, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_roller_set_options(this->roller_tens, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(this->roller_tens, 2);
    lv_obj_add_flag(this->roller_tens, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_roller_set_options(this->roller_tenths, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(this->roller_tenths, 2);
    lv_obj_add_flag(this->roller_tenths, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_obj_add_event_cb(this->container, osmi_roller_event_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
}
