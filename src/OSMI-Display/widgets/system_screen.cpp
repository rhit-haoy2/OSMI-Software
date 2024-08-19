/**
 * @file system_screen.cpp
 * @author Wei Huang & Jake Armstrong
 * @brief The "Home" screen for the OSMI pump user interface.
 * @version 0.1
 * @date 2024-02-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "system_screen.h"

/// @brief 
/// @param event 
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


static void resetinfufast_button_handler(lv_event_t *event){
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_PRESSED){
        screen->Driver->disable();
        screen->Driver->setDirection(Reverse);
        if (screen->Driver->setVelocity(40) == ESP_OK){
            screen->Driver->enable();
        }
    }else if(code == LV_EVENT_RELEASED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        
    }
    
}

static void resetinfuslow_button_handler(lv_event_t *event){
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_PRESSED){
        screen->Driver->disable();
        screen->Driver->setDirection(Reverse);
        if (screen->Driver->setVelocity(1) == ESP_OK){
            screen->Driver->enable();
        }
        
    }else if(code == LV_EVENT_RELEASED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        
    }
    
}

static void forwardinfufast_button_handler(lv_event_t *event){
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_PRESSED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        if (screen->Driver->setVelocity(40) == ESP_OK){
            screen->Driver->enable();
        }
        
    }else if(code == LV_EVENT_RELEASED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        
    }
    
}

static void forwardinfuslow_button_handler(lv_event_t *event){
    system_screen_t *screen = (system_screen_t *)lv_obj_get_user_data(event->target);
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_PRESSED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        screen->Driver->setVelocity(1);
        screen->Driver->enable();
    }else if(code == LV_EVENT_RELEASED){
        screen->Driver->disable();
        screen->Driver->setDirection(Depress);
        
    }
    
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
    lv_obj_t *dossettings_container = lv_obj_create(screen->system_screen);
    lv_obj_set_flex_flow(dossettings_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(dossettings_container, 230, 70);

    temporary_label = lv_label_create(dossettings_container);
    lv_label_set_text(temporary_label, "Dosage Settings: ");


    screen->dossetEdit_button = lv_btn_create(dossettings_container);
    temporary_label = lv_label_create(screen->dossetEdit_button);
    lv_label_set_text(temporary_label, "Edit");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->dossetEdit_button,screen);
    lv_obj_add_event_cb(screen->dossetEdit_button, dossetEdit_button_handler, LV_EVENT_RELEASED, screen);

    screen->resetinfufast_button = lv_btn_create(screen->system_screen);
    temporary_label = lv_label_create(screen->resetinfufast_button);
    lv_label_set_text(temporary_label, "ResetSyringeFast");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->resetinfufast_button,screen);
    lv_obj_add_event_cb(screen->resetinfufast_button, resetinfufast_button_handler,LV_EVENT_ALL,screen);

    screen->resetinfuslow_button = lv_btn_create(screen->system_screen);
    temporary_label = lv_label_create(screen->resetinfuslow_button);
    lv_label_set_text(temporary_label, "ResetSyringeSlow");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->resetinfuslow_button,screen);
    lv_obj_add_event_cb(screen->resetinfuslow_button, resetinfuslow_button_handler,LV_EVENT_ALL,screen);

    screen->forwardinfufast_button = lv_btn_create(screen->system_screen);
    temporary_label = lv_label_create(screen->forwardinfufast_button);
    lv_label_set_text(temporary_label, "ForwardSyringeFast");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->forwardinfufast_button,screen);
    lv_obj_add_event_cb(screen->forwardinfufast_button, forwardinfufast_button_handler,LV_EVENT_ALL,screen);
    

    screen->forwardinfuslow_button = lv_btn_create(screen->system_screen);
    temporary_label = lv_label_create(screen->forwardinfuslow_button);
    lv_label_set_text(temporary_label, "ForwardSyringeSlow");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->forwardinfuslow_button,screen);
    lv_obj_add_event_cb(screen->forwardinfuslow_button, forwardinfuslow_button_handler,LV_EVENT_ALL,screen);
}