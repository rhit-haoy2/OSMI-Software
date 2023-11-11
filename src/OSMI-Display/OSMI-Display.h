/**OSMI Display Task for rendering UI things.*/

#ifndef __OSMI_DISPLAY_H
#define __OSMI_DISPLAY_H
#include <PNGdec.h>
#include "rose_logo.h" // Image is stored here in an 8 bit array
// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>              // Hardware-specific library

#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

#define ROSE_LOGO_WIDTH 510


void setupDisplay(void);
void pngDraw(PNGDRAW *pDraw);
void loopDisplay(byte status);
void DisplayTask( void *params );

#endif