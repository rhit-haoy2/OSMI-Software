#include "OSMI-Display.h"
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "TFT_Config.h"
#include "OSMI-Control/FluidDeliveryController.h"
#include "./widgets/config_screen.h"
#include "./widgets/status_screen.h"
#include "./widgets/system_screen.h"

#define SPI_DRIVER_CS 27
#define MOTOR_PWM_PIN 26
#define LIMIT_SWITCH_PIN 25
#define PITCH 8.0
#define DEG_PER_STEP 0.9
#define volumePerDistance 0.66667

TFT_eSPI tft = TFT_eSPI();
static ESP32PwmSpiDriver driverInst = ESP32PwmSpiDriver(SPI_DRIVER_CS, MOTOR_PWM_PIN, LIMIT_SWITCH_PIN, PITCH, DEG_PER_STEP);
static Team11Control controller = Team11Control(1, (FluidDeliveryDriver *)&driverInst);

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

    driverInst.initialize();
    tft.begin();
    tft.init();
    uint16_t calData[5] = {531, 3290, 415, 3480, 6};
    tft.setTouch(calData);

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
    status_screen_t status_screen;
    system_screen_t system_screen;

    status_screen.controller = &controller;
    status_screen.config_screen = &config_screen;
    create_status_screen(&status_screen);

    config_screen.controller = &controller;
    config_screen.status_screen = status_screen.status_screen;
    config_screen.timer= status_screen.timer;

    create_config_screen(&config_screen); // temporarily load config screen as default screen.

    system_screen.Driver = &driverInst;
    system_screen.config_screen = config_screen.config_screen;
    system_screen.status_screen = status_screen.status_screen;
    create_system_screen(&system_screen);

    config_screen.system_screen = system_screen.system_screen;

    lv_scr_load(system_screen.system_screen);

    while (true)
    {
        lv_timer_handler();

        delay(50);
    }
}
