#include "OSMI-Display.h"
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "TFT_Config.h"
#include "OSMI-Control/FluidDeliveryController.h"
#include "./widgets/config_screen.h"

#define SPI_DRIVER_CS 27
#define MOTOR_PWM_PIN 26
#define LIMIT_SWITCH_PIN 25
#define DIST_PER_STEP 1.0

TFT_eSPI tft = TFT_eSPI();
static Team11Control *controller;

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

// Parameter of the rate.
int flowrate;
int unit;

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

    if (code == LV_EVENT_CLICKED)
    {

        controller->stopFlow();

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text(statusLabel, "Paused");
    }
}

static void btn_event_Start(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {

        controller->startFlow();

        /*Get the first child of the button which is the label and change its text*/
        lv_label_set_text(statusLabel, "Delivering");
    }
}

static void UpdateFlowText()
{
    controller->stopFlow();
    std::string flowtext = "Rate: ";
    char num[5];
std:
    sprintf(num, "%d ", flowrate);
    flowtext += num;
    switch (unit)
    {
    case 0:
        flowtext += "ml/sec";
        break;
    case 1:
        flowtext += "ml/min";
        break;
    case 2:
        flowtext += "ml/h";
        break;
    default:
        flowtext += "ml/sec";
        break;
    }
    lv_label_set_text(rateLabel, flowtext.c_str());
}

static void UpdateRate(){
    float rate = flowrate;
    switch (unit)
    {
    case 0:
        rate = rate*60;
        break;
    case 1:
        rate = rate;
        break;
    case 2:
        rate = rate/60.0;
        break;
    default:
        break;
    }
    controller->setFlow(rate);
}

// static void btn_event_UpdateRate(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t *btn = lv_event_get_target(e);
//     if (code == LV_EVENT_CLICKED)
//     {
//         if (controller != nullptr)
//         {
//             controller->stopFlow();
//             controller->setFlow(20);
//             lv_obj_t *label = lv_obj_get_child(btn, 0);
//             lv_label_set_text(rateLabel, lv_label_get_text(label));
//         }
//     }
// }

static void roller_event_UpdateRateHun(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int hundreds = lv_roller_get_selected(obj);
        flowrate = (flowrate % 100) + (hundreds * 100);
        UpdateFlowText();
    }
}

static void roller_event_UpdateRateTen(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int tens = lv_roller_get_selected(obj);
        flowrate = ((flowrate / 100) * 100) + tens * 10 + (flowrate % 10);
        UpdateFlowText();
    }
}

static void roller_event_UpdateRateOne(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int ones = lv_roller_get_selected(obj);
        flowrate = ((flowrate / 10) * 10) + ones;
        UpdateFlowText();
    }
}

static void roller_event_UpdateRateUnit(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        unit = lv_roller_get_selected(obj);
        Serial.println(flowrate);
        UpdateFlowText();
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

/// @brief Task function for display subsystem.
/// @param params DisplayConfig_t
void DisplayTask(void *params)
{

    tft.begin();
    /*TODO MOVE TO POST*/
    // readSetup(tft);
    /*END TODO*/
    tft.init();

    // touch_calibrate();

    uint16_t calData[5] = {531, 3290, 415, 3480, 6};
    tft.setTouch(calData);

    // Setup parameters.
    display_config_t *handle = (display_config_t *)params;

    FluidDeliveryDriver *driverInst = (FluidDeliveryDriver *)new ESP32PwmSpiDriver(SPI_DRIVER_CS, MOTOR_PWM_PIN, LIMIT_SWITCH_PIN, DIST_PER_STEP);
    controller = new Team11Control(1, driverInst);
    // Serial.printf("HandleDispAddress: %p\n", handle);
    // Serial.printf("DriverDispAddress: %p\n", handle->driver);
    // Serial.printf("QueueDispAddress: %p\n", handle->handle);

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


    config_screen_t config_screen;
    config_screen.controller = controller;
    config_screen.status_screen = NULL;
    create_config_screen(&config_screen); // temporarily load config screen as default screen.

    lv_scr_load(config_screen.config_screen);
    
    while (true)
    {
        ((ESP32PwmSpiDriver*)controller->getDriver())->occlusionDetected();
        lv_timer_handler();
        delay(15);
    }
}
