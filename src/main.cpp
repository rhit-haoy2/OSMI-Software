#include <Arduino.h>
/*
  There are four different methods of plotting anti-aliased fonts to the screen.

  This sketch uses method 1, using tft.print() and tft.println() calls.

  In some cases the sketch shows what can go wrong too, so read the comments!

  The font is rendered WITHOUT a background, but a background colour needs to be
  set so the anti-aliasing of the character is performed correctly. This is because
  characters are drawn one by one.

  This method is good for static text that does not change often because changing
  values may flicker. The text appears at the tft cursor coordinates.

  It is also possible to "print" text directly into a created sprite, for example using
  spr.println("Hello"); and then push the sprite to the screen. That method is not
  demonstrated in this sketch.

*/
//  The fonts used are in the sketch data folder, press Ctrl+K to view.

//  Upload the fonts and icons to SPIFFS (must set at least 1M for SPIFFS) using the
//  "Tools"  "ESP8266 (or ESP32) Sketch Data Upload" menu option in the IDE.
//  To add this option follow instructions here for the ESP8266:
//  https://github.com/esp8266/arduino-esp8266fs-plugin
//  or for the ESP32:
//  https://github.com/me-no-dev/arduino-esp32fs-plugin

//  Close the IDE and open again to see the new menu option.

//  A processing sketch to create new fonts can be found in the Tools folder of TFT_eSPI
//  https://github.com/Bodmer/TFT_eSPI/tree/master/Tools/Create_Smooth_Font/Create_font

//  This sketch uses font files created from the Noto family of fonts:
//  https://www.google.com/get/noto/

#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

#define ROSE_LOGO_WIDTH 510

#include <PNGdec.h>
#include "rose_logo.h" // Image is stored here in an 8 bit array

PNG png; // PNG decoder inatance

int16_t xpos = 50;
int16_t ypos = 150;

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

void setup(void)
{

  Serial.begin(115200);

  tft.begin();

  tft.setRotation(0);

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS available!");

  // ESP32 will crash if any of the fonts are missing
  bool font_missing = false;
  if (SPIFFS.exists("/NotoSansBold15.vlw") == false)
    font_missing = true;
  if (SPIFFS.exists("/NotoSansBold36.vlw") == false)
    font_missing = true;

  if (font_missing)
  {
    Serial.println("\r\nFont missing in SPIFFS, did you upload it?");
    while (1)
      yield();
  }
  else
    Serial.println("\r\nFonts found OK.");
}

void pngDraw(PNGDRAW *pDraw)
{
  uint16_t lineBuffer[ROSE_LOGO_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void loop()
{

  tft.loadFont(AA_FONT_SMALL); // Must load the font first

  // Get ready for the next demo while we have this font loaded
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0); // Set cursor at top left of screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Device has been on for:");

  tft.unloadFont(); // Remove the font to recover memory used

  // IMAGE
  int16_t rc = png.openFLASH((uint8_t *)rose_logo, sizeof(rose_logo), pngDraw);
  String errorCode;
  switch (rc)
  {
  case PNG_INVALID_PARAMETER:
    errorCode = "Invalid Param";
    break;
  case PNG_DECODE_ERROR:
    errorCode = "Decode error.";
    break;
  case PNG_MEM_ERROR:
    errorCode = "Mem Error";
    break;
  case PNG_NO_BUFFER:
    errorCode = "No Buffer";
    break;
  default:
    errorCode = "Other Error " + String(rc);
  }
  Serial.println(errorCode);

  if (rc == PNG_SUCCESS)
  {
    Serial.println("Successfully opened png file");
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    tft.startWrite();
    uint32_t dt = millis();
    rc = png.decode(NULL, 0);
    Serial.print(millis() - dt);
    Serial.println("ms");
    tft.endWrite();
    png.close(); // not needed for memory->memory decode
  }

  delay(5000);
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Large font
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  // tft.fillScreen(TFT_BLACK);

  // Draw changing numbers - does not work unless a filled rectangle is drawn over the old text
  while (true)
  {
    // Adding a parameter "true" to the setTextColor() function fills character background
    // This extra parameter is only for smooth fonts!
    tft.loadFont(AA_FONT_LARGE); // Load another different font
    tft.setTextColor(TFT_GREEN, TFT_BLACK, true);
    tft.setCursor(30, 30);
    tft.print(millis() / 1000.0, 1);
    tft.unloadFont();
    // delay (200);

    tft.loadFont(AA_FONT_SMALL);
    tft.setCursor(0, 80);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Device Status:");
    tft.unloadFont();
    tft.loadFont(AA_FONT_SMALL);
    tft.setTextColor(TFT_GREEN, TFT_BLACK, true);
    tft.setCursor(30, 110);
    tft.println("STEPPER 1 ON");
  }
}
