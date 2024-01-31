#include "status_screen.h"
#include "config_screen.h"


static void pause_button_handler(lv_event_t *event)
{
    status_screen_t *screen = (status_screen_t *)lv_obj_get_user_data(event->target);
    screen->controller->stopFlow();
    Serial.println("Pause Button Pressed.");
}



void create_status_screen(status_screen_t *screen)
{
    lv_obj_t *temporary_label;





    screen->status_screen = lv_obj_create(NULL);
    // configure screen layout.
    lv_obj_set_layout(screen->status_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen->status_screen, LV_FLEX_FLOW_COLUMN);

    //title
    temporary_label = lv_label_create(screen->status_screen);
    lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, &lv_font_montserrat_36);
    lv_obj_add_style(temporary_label, &title_style, 0);
    lv_label_set_text(temporary_label, "Pumping Info");


    //Get params First.
    float currentvolume = screen->controller->getVolumeDelivered();
    float bolusvolume = config_screen_get_bolus_volume(screen->config_screen);
    float infuvolume = config_screen_get_infusion_volume(screen->config_screen);
    float bolusrate = config_screen_get_bolus_rate(screen->config_screen);
    float infurate = config_screen_get_infusion_rate(screen->config_screen);




    temporary_label = lv_label_create(screen->status_screen);
    std::string volumetext = "Current Rate: ";
    char num[5];
std:
    sprintf(num, "%f ",bolusrate);
    volumetext += num;
    lv_label_set_text(temporary_label, volumetext.c_str());

    
    
    temporary_label = lv_label_create(screen->status_screen);
    std::string timetext = "Time Left: estimate";
    float timeleft = (bolusvolume/bolusrate) + (infuvolume/infurate);
    sprintf(num, "%d ",timeleft);
    volumetext += num;
    sprintf(num, "sec");
    volumetext += num;
    lv_label_set_text(temporary_label, volumetext.c_str());

    temporary_label = lv_label_create(screen->status_screen);
    lv_label_set_text(temporary_label, "Bolus: ");


    lv_obj_set_flex_flow(temporary_label, LV_FLEX_FLOW_ROW_REVERSE);

    temporary_label = lv_label_create(screen->status_screen);
    lv_label_set_text(temporary_label, "Infu: ");
    lv_obj_set_flex_flow(temporary_label, LV_FLEX_FLOW_COLUMN);

    lv_obj_t * bar1 = lv_bar_create(screen->status_screen);
    lv_obj_set_flex_flow(temporary_label, LV_FLEX_FLOW_ROW_REVERSE);
    lv_obj_set_size(bar1, 20, 200);
    lv_bar_set_value(bar1, 100, LV_ANIM_ON);
    lv_bar_set_range(bar1, 0, 100);
    float percent = currentvolume/bolusvolume;
    lv_bar_set_value(bar1,percent,LV_ANIM_ON);

    lv_obj_t * bar2 = lv_bar_create(screen->status_screen);
    lv_obj_set_flex_flow(temporary_label, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(bar2, 20, 200);
    lv_bar_set_value(bar2, 100, LV_ANIM_ON);
    lv_bar_set_range(bar2, 0, 100);
    percent = 0.0;
    lv_bar_set_value(bar2,percent,LV_ANIM_ON);


    lv_obj_t * pausebtn = lv_btn_create(screen->status_screen);
    temporary_label = lv_label_create(pausebtn);
    lv_label_set_text(temporary_label, "Pause");
    lv_obj_center(temporary_label);
    lv_obj_add_event_cb(pausebtn, pause_button_handler, LV_EVENT_RELEASED, screen);











}


