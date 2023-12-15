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

    QueueHandle_t *handle = (QueueHandle_t *)params;
    while (true)
    {
        lv_timer_handler();
        delay(15);
    }
}
