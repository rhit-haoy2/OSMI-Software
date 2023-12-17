#include "OSMI-Display.h"
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "demos/lv_demos.h"

TFT_eSPI tft = TFT_eSPI();

/*Input device driver descriptor*/
static lv_indev_t indev_drv;

/*Display Driver Descripter*/
static lv_disp_drv_t display;

/* Descriptor of a display driver*/
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t disp_buf[DISPLAY_VERT * DISPLAY_HORZ / 10];

void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);

    lv_obj_t * label = lv_obj_get_child(slider, 0);
    /*Refresh the text*/
    lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
}

void DisplayTask(void *params)
{

    tft.begin();

    lv_init();

    lv_disp_draw_buf_init(&draw_buffer, disp_buf, NULL, DISPLAY_VERT * DISPLAY_HORZ / 10);

    lv_disp_drv_init(&display);
    display.hor_res = TFT_WIDTH;
    display.ver_res = TFT_HEIGHT;
    display.flush_cb = display_flush;
    display.draw_buf = &draw_buffer;
    lv_disp_drv_register(&display);

    lv_obj_t *label = lv_label_create( lv_scr_act() );
    lv_label_set_text( label, "Hello Ardino and LVGL!");
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * btnlabel = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(btnlabel, "Button");                     /*Set the labels text*/
    lv_obj_center(btnlabel);


    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_set_width(slider, 200);                          /*Set the width*/
    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

    /*Create a label above the slider*/
    lv_obj_t *sliderlabel = lv_label_create(slider);
    lv_label_set_text(sliderlabel, "0");
    lv_obj_align_to(sliderlabel, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/


    QueueHandle_t *handle = (QueueHandle_t *)params;
    while (true)
    {
        lv_timer_handler();
        delay(15);
    }
}
