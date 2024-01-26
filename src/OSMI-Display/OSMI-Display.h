/**OSMI Display Task for rendering UI things.*/

#ifndef __OSMI_DISPLAY_H
#define __OSMI_DISPLAY_H
// Include TFT Configuration header
// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h> // Hardware-specific library
#include "../OSMI-Control/FluidDeliveryController.h"
#include <Arduino.h>
#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

#define DISPLAY_VERT 320
#define DISPLAY_HORZ 240

typedef struct
{
    // FluidDeliveryDriver* driver;
    QueueHandle_t *handle;
} display_config_t;

void DisplayTask(void *params);

#endif