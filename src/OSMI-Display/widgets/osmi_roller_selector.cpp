#include "osmi_roller_selector.h"
#include <stdio.h>

/**
 * @brief Handles when any of the values change.
 *
 * @param event LVGL Event object.
 */
static void osmi_roller_event_handler(lv_event_t *event)
{
    osmi_roller_selector *selector = (osmi_roller_selector *)lv_event_get_user_data(event);
    if (selector == NULL)
        return;
    float tens = lv_roller_get_selected(selector->roller_tens);
    float ones = lv_roller_get_selected(selector->roller_ones);
    float tenths = lv_roller_get_selected(selector->roller_tenths);

    selector->value = (tens * 10) + ones + (tenths * 0.1);

    if (lv_roller_get_option_cnt(selector->roller_units) > 1) {
        switch(lv_roller_get_selected(selector->roller_units)) {
            case 0: // ml/sec
                selector->value = selector->value * 60.0f;
                break;
            case 2: // ml / hr
                selector->value = selector->value / 60.0f;
                break;
            default:
                break;
        }
    }
}

float osmi_roller_get_value(osmi_roller_selector *selector)
{
    return selector->value;
}

void osmi_roller_selector_create(lv_obj_t *parent, osmi_roller_selector *selector, const char *numeric_options, const char *unit_options)
{
    selector->value = 0.0;

    selector->container = lv_obj_create(parent);
    lv_obj_set_size(selector->container, 180, 100);
    // Container flex allign settings.
    lv_obj_set_flex_flow(selector->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(selector->container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_scrollbar_mode(selector->container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(selector->container, LV_OBJ_FLAG_SCROLLABLE);

    selector->roller_tens = lv_roller_create(selector->container);
    selector->roller_ones = lv_roller_create(selector->container);

    lv_obj_t *decimal = lv_label_create(selector->container);
    lv_label_set_text(decimal, ".");

    selector->roller_tenths = lv_roller_create(selector->container);
    selector->roller_units = lv_roller_create(selector->container);

    // Units roller settings.
    lv_roller_set_options(selector->roller_units, unit_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(selector->roller_units, 2);
    lv_obj_add_flag(selector->roller_units, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    // Numeric value settings.
    lv_roller_set_options(selector->roller_ones, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(selector->roller_ones, 2);
    lv_obj_add_flag(selector->roller_ones, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_roller_set_options(selector->roller_tens, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(selector->roller_tens, 2);
    lv_obj_add_flag(selector->roller_tens, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_roller_set_options(selector->roller_tenths, numeric_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(selector->roller_tenths, 2);
    lv_obj_add_flag(selector->roller_tenths, LV_OBJ_FLAG_EVENT_BUBBLE); // required for proper event handling.

    lv_obj_add_event_cb(selector->container, osmi_roller_event_handler, LV_EVENT_VALUE_CHANGED, (void *)selector);
}
