/**OSMI Display Task for rendering UI things.*/

#ifndef __OSMI_DISPLAY_H
#define __OSMI_DISPLAY_H
// Include TFT Configuration header
// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>              // Hardware-specific library
#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

#define DISPLAY_VERT 320
#define DISPLAY_HORZ 240

void DisplayTask( void *params );

#endif