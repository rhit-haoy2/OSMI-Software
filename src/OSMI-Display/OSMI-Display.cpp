#include "OSMI-Display.h"
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "demos/lv_demos.h"
#include "TFT_Config.h"

TFT_eSPI tft = TFT_eSPI();
static FluidDeliveryController *controller;

/*Input device driver descriptor*/
static lv_indev_t *my_indev;

/*Display Driver Descripter*/
static lv_disp_drv_t display;
static lv_indev_drv_t indev_drv;

/* Descriptor of a display driver*/
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t disp_buf[DISPLAY_VERT * DISPLAY_HORZ / 10];

// Label for current rate.
static lv_obj_t *rateLabel;

// Label for delivering state.
static lv_obj_t *statusLabel;

/// @brief Flushes draw buffer to the display.
/// @param disp
/// @param area
/// @param color_p
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

static void btn_event_Pause(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    FluidDeliveryController *controllerPointer = (FluidDeliveryController *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        // Please get rid of count.
        static uint8_t cnt = 0;
        cnt++;

        controllerPointer->stopFlow();

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Stop: %d", cnt);
        lv_label_set_text(statusLabel, "Paused");
    }
}

static void btn_event_Start(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    FluidDeliveryController *controllerPointer = (FluidDeliveryController *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {

        controllerPointer->startFlow();

        /*Get the first child of the button which is the label and change its text*/
        lv_label_set_text(statusLabel, "Delivering");
    }
}

static void btn_event_UpdateRate(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (controller != nullptr)
        {
            controller->stopFlow();
            controller->setFlow(20);
            lv_obj_t *label = lv_obj_get_child(btn, 0);
            lv_label_set_text(rateLabel, lv_label_get_text(label));
        }
    }
}

static void scroll_event_cb(lv_event_t *e)
{
    lv_obj_t *cont = lv_event_get_target(e);

    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for (i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if (diff_y >= r)
        {
            x = r;
        }
        else
        {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }

        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, x, 0);

        /*Use some opacity with larger translations*/
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);

        /*if(x==0){
          lv_label_set_text(rrlabel, lv_label_get_text(lv_obj_get_child(child,0)));
        }*/
    }
}

/// @brief Read touch input data from touchscreen.
/// @param drv Touch driver.
/// @param data Data for touch.
static void touchInputRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t x, y;
    if (tft.getTouch(&x, &y))
    {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;

        Serial.print("x,y = ");
        Serial.print(x);
        Serial.print(",");
        Serial.println(y);
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/**
 * @brief Get calibration data for touchscreen.
 * @author Bodmer
 */
void touch_calibrate()
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // Calibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println();
    Serial.println("// Use this calibration code in setup():");
    Serial.print("  uint16_t calData[5] = ");
    Serial.print("{ ");

    for (uint8_t i = 0; i < 5; i++)
    {
        Serial.print(calData[i]);
        if (i < 4)
            Serial.print(", ");
    }

    Serial.println(" };");
    Serial.print("  tft.setTouch(calData);");
    Serial.println();
    Serial.println();

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    tft.println("Calibration code sent to Serial port.");

    delay(4000);
}

void DisplayTask(void *params)
{

    tft.begin();
    /*TODO MOVE TO POST*/
    readSetup(tft);
    /*END TODO*/
    tft.init();

    // touch_calibrate();

    uint16_t calData[5] = {531, 3290, 415, 3480, 6};
    tft.setTouch(calData);

    // Setup parameters.
    display_config_t *handle = (display_config_t *)params;
    if (handle->controller == nullptr)
    {
        Serial.println("Display Controller Null!");
        while (1)
            ; // NUll pointer check.
    }
    controller = handle->controller;

    lv_init();

    lv_disp_t *disp = NULL;
    lv_theme_t *th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    lv_disp_draw_buf_init(&draw_buffer, disp_buf, NULL, DISPLAY_VERT * DISPLAY_HORZ / 10);

    lv_disp_drv_init(&display);
    display.hor_res = TFT_WIDTH;
    display.ver_res = TFT_HEIGHT;
    display.flush_cb = display_flush;
    display.draw_buf = &draw_buffer;
    lv_disp_drv_register(&display);

    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = touchInputRead;     /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    my_indev = lv_indev_drv_register(&indev_drv);

    lv_obj_t *startbtn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(startbtn, 10, 10);  /*Set its position*/
    lv_obj_set_size(startbtn, 80, 50); /*Set its size*/
    lv_obj_add_event_cb(startbtn, btn_event_Start, LV_EVENT_ALL, controller);
    lv_obj_t *startbtnlabel = lv_label_create(startbtn); /*Add a label to the button*/
    lv_label_set_text(startbtnlabel, "Start");           /*Set the labels text*/
    lv_obj_center(startbtnlabel);

    lv_obj_t *btn = lv_btn_create(lv_scr_act());                         /*Add a button the current screen*/
    lv_obj_set_pos(btn, 100, 10);                                        /*Set its position*/
    lv_obj_set_size(btn, 80, 50);                                        /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_Pause, LV_EVENT_ALL, controller); /*Assign a callback to the button*/
    lv_obj_t *btnlabel = lv_label_create(btn);                           /*Add a label to the button*/
    lv_label_set_text(btnlabel, "Stop");                                 /*Set the labels text*/
    lv_obj_center(btnlabel);

    statusLabel = lv_label_create(lv_scr_act());
    lv_obj_set_pos(statusLabel, 50, 50);
    lv_obj_set_size(statusLabel, 80, 50);
    lv_label_set_text(statusLabel, "Paused");

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 200, 200);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    /*lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);*/

    uint32_t i;
    for (i = 0; i < 10; i++)
    {
        lv_obj_t *btn = lv_btn_create(cont);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_add_event_cb(btn, btn_event_UpdateRate, LV_EVENT_ALL, NULL);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Rate %" LV_PRIu32, i);
    }

    /*Update the buttons position manually for first*/
    lv_event_send(cont, LV_EVENT_SCROLL, NULL);

    /*Be sure the fist button is in the middle*/
    lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);

    rateLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(rateLabel, "Rate: 0");
    lv_obj_align_to(rateLabel, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
    lv_obj_align_to(statusLabel, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

    while (true)
    {

        lv_timer_handler();
        delay(15);
    }
}
