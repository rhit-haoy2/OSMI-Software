/**
 * @file osmi_roller_selector.h
 * @author Jake Armstrong
 * @brief OSMI Roller Selector for 3 digits and one unit box.
 * @version 0.1
 * @date 2024-01-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef roller_selector_h
#define roller_selector_h
#include <lvgl.h>
#include <core/lv_obj_class.h>

typedef struct
{
    lv_obj_t *container;

    lv_obj_t *roller_tens;
    lv_obj_t *roller_ones;
    lv_obj_t *roller_tenths;

    lv_obj_t *roller_units;

    float value; // Value in mlPerMinute
} osmi_roller_selector;

void osmi_roller_selector_create(lv_obj_t *parent, osmi_roller_selector *selector, const char *numeric_options, const char *unit_options);
float osmi_roller_get_value(osmi_roller_selector *selector);
#endif