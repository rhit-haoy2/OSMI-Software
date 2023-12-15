/**OSMI Display Task for rendering UI things.*/

#ifndef __OSMI_DISPLAY_H
#define __OSMI_DISPLAY_H
// Include TFT Configuration header
// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>              // Hardware-specific library
#include "lvgl.h"

#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

#define ROSE_LOGO_WIDTH 510
#define DISPLAY_VERT 320
#define DISPLAY_HORZ 240

void DisplayTask( void *params );   
void display_flush(lv_disp_t* disp, const lv_area_t* area, lv_color_t* color_p);

#endif