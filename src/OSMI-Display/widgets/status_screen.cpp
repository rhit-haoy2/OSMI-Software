#include "status_screen.h"
#include "config_screen.h"


static void pause_button_handler(lv_event_t *event)
{
    status_screen_t *screen = (status_screen_t *)lv_obj_get_user_data(event->target);
    screen->controller->stopFlow();
    Serial.println("Pause Button Pressed.");
    lv_scr_load(screen->config_screen->config_screen);
    lv_timer_pause(screen->timer);
}

void my_timer(lv_timer_t * timer)
{
    /*Use the user_data*/
    status_screen_t * screen = (status_screen_t *)timer->user_data;

    float currentvolume = screen->controller->getVolumeDelivered();
    float bolusvolume = config_screen_get_bolus_volume(screen->config_screen);
    float infuvolume = config_screen_get_infusion_volume(screen->config_screen);
    float bolusrate = config_screen_get_bolus_rate(screen->config_screen);
    float infurate = config_screen_get_infusion_rate(screen->config_screen);

    float boluspercent = 1.0;
    float timeleft = infuvolume/infurate;
    if(bolusvolume!=0){
        boluspercent = currentvolume/bolusvolume;
        timeleft = (bolusvolume/bolusrate) + ((infuvolume-bolusvolume)/infurate);
    }
    float infupercent = currentvolume/infuvolume;

    
    /*Do something with LVGL*/
    if(boluspercent<1){
        boluspercent = boluspercent*100;
        infupercent = infupercent*100;
        int i1 = round(boluspercent);
        int i2 = round(infupercent);
        lv_bar_set_value(screen->bolus_bar,i1,LV_ANIM_ON);
        lv_bar_set_value(screen->infusion_bar,i2,LV_ANIM_ON);

        std::string ratetext = "Current Rate: ";
        char num[32];
        std:
        sprintf(num, "%.2f ml/h",bolusrate);
        ratetext += num;
        lv_label_set_text(screen->currentrate_text, ratetext.c_str());

        std::string timetext = "Time Left: estimate";
        timeleft = ((bolusvolume-currentvolume)/bolusrate) + ((infuvolume-bolusvolume)/infurate);
        sprintf(num, "%d sec",timeleft);
        timetext += num;
        lv_label_set_text(screen->timeleft_text, timetext.c_str());
        

    }else if(infupercent<1){
        infupercent = infupercent*100;
        int i2 = round(infupercent);
        lv_bar_set_value(screen->bolus_bar,100,LV_ANIM_ON);
        lv_bar_set_value(screen->infusion_bar,i2,LV_ANIM_ON);
        std::string ratetext = "Current Rate: ";
        char num[32];
        sprintf(num, "%.2f ml/h",infurate);
        ratetext += num;
        lv_label_set_text(screen->currentrate_text, ratetext.c_str());


        std::string timetext = "Time Left: estimate";
        timeleft = ((infuvolume-currentvolume)/infurate)*3600;
        sprintf(num, "%.1f sec",timeleft);
        timetext += num;
        lv_label_set_text(screen->timeleft_text, timetext.c_str());


    }else{
        lv_bar_set_value(screen->bolus_bar,100,LV_ANIM_OFF);
        lv_bar_set_value(screen->infusion_bar,100,LV_ANIM_OFF);
    }
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



    screen->currentrate_text = lv_label_create(screen->status_screen);
    std::string ratetext = "Current Rate: ";
    char num[5];
    std:
    sprintf(num, "%.2f ml/h",bolusrate);
    ratetext += num;
    lv_label_set_text(screen->currentrate_text, ratetext.c_str());

    
    
    screen->timeleft_text = lv_label_create(screen->status_screen);
    std::string timetext = "Time Left: estimate";
    float timeleft = (bolusvolume/bolusrate) + ((infuvolume-bolusvolume)/infurate);
    sprintf(num, "%.1f sec",timeleft*3600);
    timetext += num;
    lv_label_set_text(screen->timeleft_text, timetext.c_str());


    lv_obj_t * all_bar_container = lv_obj_create(screen->status_screen);
    lv_obj_set_flex_flow(all_bar_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(all_bar_container, 230, 250);

    lv_obj_t * bolus_bar_container = lv_obj_create(all_bar_container);
    lv_obj_set_flex_flow(bolus_bar_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(bolus_bar_container, 90, 220);

    lv_obj_t * infusion_bar_container = lv_obj_create(all_bar_container);
    lv_obj_set_flex_flow(infusion_bar_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(infusion_bar_container, 100, 220);


    temporary_label = lv_label_create(bolus_bar_container);
    lv_label_set_text(temporary_label, "Bolus: ");



    temporary_label = lv_label_create(infusion_bar_container);
    lv_label_set_text(temporary_label, "Infusion: ");



    screen->bolus_bar = lv_bar_create(bolus_bar_container);
    lv_obj_set_size(screen->bolus_bar, 40, 140);
    lv_bar_set_value(screen->bolus_bar, 100, LV_ANIM_ON);
    lv_bar_set_range(screen->bolus_bar, 0, 100);
    float percent = 0.0;
    lv_bar_set_value(screen->bolus_bar,percent,LV_ANIM_ON);

    screen->infusion_bar = lv_bar_create(infusion_bar_container);
    lv_obj_set_size(screen->infusion_bar, 40, 140);
    lv_bar_set_value(screen->infusion_bar, 100, LV_ANIM_ON);
    lv_bar_set_range(screen->infusion_bar, 0, 100);
    percent = 0.0;
    lv_bar_set_value(screen->infusion_bar,percent,LV_ANIM_ON);


    screen->pause_button = lv_btn_create(screen->status_screen);
    temporary_label = lv_label_create(screen->pause_button);
    lv_label_set_text(temporary_label, "Pause");
    lv_obj_center(temporary_label);
    lv_obj_set_user_data(screen->pause_button,screen);
    lv_obj_add_event_cb(screen->pause_button, pause_button_handler, LV_EVENT_RELEASED, screen);



    static int user_data = 10;
    screen->timer = lv_timer_create(my_timer, 1000,  screen);
    lv_timer_pause(screen->timer);
    





}


