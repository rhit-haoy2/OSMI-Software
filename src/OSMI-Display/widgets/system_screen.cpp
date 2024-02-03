#include "system_screen.h"

static void syssetEdit_button_handler(lv_event_t *event)
{
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_scr_load(screen->config_screen);

}

static void dossetEdit_button_handler(lv_event_t *event)
{
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_scr_load(screen->config_screen);

}

static void resetinfu_button_handler(lv_event_t *event){
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    screen->controller
}

void create_system_screen(system_screen_t *screen){
    lv_obj_t *temporary_label;

    screen->system_screen = lv_obj_create(NULL);
    // configure screen layout.
    lv_obj_set_layout(screen->system_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen->system_screen, LV_FLEX_FLOW_COLUMN);

    //title
    temporary_label = lv_label_create(screen->system_screen);
    lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, &lv_font_montserrat_36);
    lv_obj_add_style(temporary_label, &title_style, 0);
    lv_label_set_text(temporary_label, "OSMI Home");

    // container for entering other screens.
    lv_obj_t *systemsettings_container = lv_obj_create(screen->system_screen);
    lv_obj_set_flex_flow(systemsettings_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(systemsettings_container, 200, 30);


    temporary_label = lv_label_create(systemsettings_container);
    lv_label_set_text(temporary_label, "System Settings: ");


    screen->syssetEdit_button = lv_btn_create(systemsettings_container);
    temporary_label = lv_label_create(screen->syssetEdit_button);
    lv_label_set_text(temporary_label, "Edit");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->syssetEdit_button,screen);
    lv_obj_add_event_cb(screen->syssetEdit_button, syssetEdit_button_handler, LV_EVENT_RELEASED, screen);



    lv_obj_t *dossettings_container = lv_obj_create(screen->system_screen);
    lv_obj_set_flex_flow(dossettings_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(dossettings_container, 200, 30);

    temporary_label = lv_label_create(dossettings_container);
    lv_label_set_text(temporary_label, "Dosage Settings: ");


    screen->dossetEdit_button = lv_btn_create(dossettings_container);
    temporary_label = lv_label_create(screen->dossetEdit_button);
    lv_label_set_text(temporary_label, "Edit");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->dossetEdit_button,screen);
    lv_obj_add_event_cb(screen->dossetEdit_button, dossetEdit_button_handler, LV_EVENT_RELEASED, screen);

    screen->resetinfu_button = lv_btn_create(screen->system_screen);
    temporary_label = lv_label_create(screen->dossetEdit_button);
    lv_label_set_text(temporary_label, "ResetSyringe");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->dossetEdit_button,screen);


    

}