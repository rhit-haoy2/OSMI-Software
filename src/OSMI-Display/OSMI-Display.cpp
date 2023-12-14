#include "OSMI-Display.h"
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>


TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t disp_buf[DISPLAY_VERT*DISPLAY_HORZ /10];

void DisplayTask(void *params)
{
    setupDisplay();
    lv_disp_draw_buf_init(&draw_buffer, disp_buf, NULL, DISPLAY_VERT*DISPLAY_HORZ /10);
    QueueHandle_t *handle = (QueueHandle_t *)params;
    while (true)
    {
        loopDisplay(handle);
    }
}

void display_flush(lv_disp_t *disp, const lv_area_t* area, lv_color_t color_p) {

}

void setupDisplay(void)
{

    tft.begin();
    tft.setRotation(0);

    lv_init();

    
}

void loopDisplay(QueueHandle_t *queue)
{
    lv_tick_inc(15);
}

