/**
 * @file TFT_Config.cpp
 * @author Bodmer
 * @brief Borrowed from TFT_eSPI repository for testing / diagnostic purposes.
 *
The original starting point for this library was the Adafruit_ILI9341
library in January 2015.

The licence for that library is MIT.

The first evolution of the library that led to TFT_eSPI is recorded here:

https://www.instructables.com/id/Arduino-TFT-display-and-font-library/

Adafruit_ILI9341 ORIGINAL LIBRARY HEADER:

vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  This is our library for the Adafruit  ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


Selected functions from the Adafruit_GFX library (as it was in 2015) have
been imported into the TFT_eSPI.cpp file and modified to improve
performance, add features and make them compatible with the ESP8266 and
ESP32.

The fonts from the Adafruit_GFX and Button functions were added later.
The fonts can be found with the license.txt file in the "Fonts\GFXFF"
folder.

The Adafruit_GFX functions are covered by the BSD licence.

Adafruit_GFX ORIGINAL LIBRARY LICENSE:

vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

Software License Agreement (BSD License)

Copyright (c) 2012 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Due to the evolution of the TFT_eSPI library the original code may no longer
be recognisable, however in most cases the function names can be used as a
reference point since the aim is to retain a level of compatibility with
the popular Adafruit_GFX graphics functions.

Contributions from other authors are recorded on GitHub:
https://github.com/Bodmer/TFT_eSPI

The major addition to the original library was the addition of fast
rendering proportional fonts of different sizes as documented here:

https://www.instructables.com/id/Arduino-TFT-display-and-font-library/

The larger fonts are "Run Length Encoded (RLE)", this was done to
reduce the font memory footprint for AVR processors that have limited
FLASH, with the added benefit of a significant improvement in rendering
speed.

In 2016 the library evolved significantly to support the ESP8266 and then
the ESP32. In 2017 new Touch Screen functions were added and a new Sprite
class called TFT_eSprite to permit "flicker free" screen updates of complex
graphics.

In 2018 anti-aliased fonts were added along with a Processing font conversion
sketch.

In 2019 the library was adapted to be able to use it with any 32-bit Arduino
compatible processor. It will run on 8-bit and 16-bit processors but will be
slow due to extensive use of 32-bit variables.

Many of the example sketches are original work that contain code created
for my own projects. For all the original code the FreeBSD licence applies
and is compatible with the GNU GPL.

vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
Software License Agreement (FreeBSD License)

Copyright (c) 2023 Bodmer (https://github.com/Bodmer)

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 */

#include "TFT_Config.h"

static setup_t user;

void printProcessorName(void)
{
    Serial.print("Processor    = ");
    if (user.esp == 0x8266)
        Serial.println("ESP8266");
    if (user.esp == 0x32)
        Serial.println("ESP32");
    if (user.esp == 0x32F)
        Serial.println("STM32");
    if (user.esp == 0x2040)
        Serial.println("RP2040");
    if (user.esp == 0x0000)
        Serial.println("Generic");
}

// Get pin name
int8_t getPinName(int8_t pin)
{
    // For ESP32 and RP2040 pin labels on boards use the GPIO number
    if (user.esp == 0x32 || user.esp == 0x2040)
        return pin;

    if (user.esp == 0x8266)
    {
        // For ESP8266 the pin labels are not the same as the GPIO number
        // These are for the NodeMCU pin definitions:
        //        GPIO       Dxx
        if (pin == 16)
            return 0;
        if (pin == 5)
            return 1;
        if (pin == 4)
            return 2;
        if (pin == 0)
            return 3;
        if (pin == 2)
            return 4;
        if (pin == 14)
            return 5;
        if (pin == 12)
            return 6;
        if (pin == 13)
            return 7;
        if (pin == 15)
            return 8;
        if (pin == 3)
            return 9;
        if (pin == 1)
            return 10;
        if (pin == 9)
            return 11;
        if (pin == 10)
            return 12;
    }

    if (user.esp == 0x32F)
        return pin;

    return pin; // Invalid pin
}

void readSetup(TFT_eSPI tft)
{
    tft.getSetup(user); //

    Serial.print("\n[code]\n");

    Serial.print("TFT_eSPI ver = ");
    Serial.println(user.version);
    printProcessorName();
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP8266)
    if (user.esp < 0x32F000 || user.esp > 0x32FFFF)
    {
        Serial.print("Frequency    = ");
        Serial.print(ESP.getCpuFreqMHz());
        Serial.println("MHz");
    }
#endif
#ifdef ARDUINO_ARCH_ESP8266
    Serial.print("Voltage      = ");
    Serial.print(ESP.getVcc() / 918.0);
    Serial.println("V"); // 918 empirically determined
#endif
    Serial.print("Transactions = ");
    Serial.println((user.trans == 1) ? "Yes" : "No");
    Serial.print("Interface    = ");
    Serial.println((user.serial == 1) ? "SPI" : "Parallel");
#ifdef ARDUINO_ARCH_ESP8266
    if (user.serial == 1)
    {
        Serial.print("SPI overlap  = ");
        Serial.println((user.overlap == 1) ? "Yes\n" : "No\n");
    }
#endif
    if (user.tft_driver != 0xE9D) // For ePaper displays the size is defined in the sketch
    {
        Serial.print("Display driver = ");
        Serial.println(user.tft_driver, HEX); // Hexadecimal code
        Serial.print("Display width  = ");
        Serial.println(user.tft_width); // Rotation 0 width and height
        Serial.print("Display height = ");
        Serial.println(user.tft_height);
        Serial.println();
    }
    else if (user.tft_driver == 0xE9D)
        Serial.println("Display driver = ePaper\n");

    if (user.r0_x_offset != 0)
    {
        Serial.print("R0 x offset = ");
        Serial.println(user.r0_x_offset);
    } // Offsets, not all used yet
    if (user.r0_y_offset != 0)
    {
        Serial.print("R0 y offset = ");
        Serial.println(user.r0_y_offset);
    }
    if (user.r1_x_offset != 0)
    {
        Serial.print("R1 x offset = ");
        Serial.println(user.r1_x_offset);
    }
    if (user.r1_y_offset != 0)
    {
        Serial.print("R1 y offset = ");
        Serial.println(user.r1_y_offset);
    }
    if (user.r2_x_offset != 0)
    {
        Serial.print("R2 x offset = ");
        Serial.println(user.r2_x_offset);
    }
    if (user.r2_y_offset != 0)
    {
        Serial.print("R2 y offset = ");
        Serial.println(user.r2_y_offset);
    }
    if (user.r3_x_offset != 0)
    {
        Serial.print("R3 x offset = ");
        Serial.println(user.r3_x_offset);
    }
    if (user.r3_y_offset != 0)
    {
        Serial.print("R3 y offset = ");
        Serial.println(user.r3_y_offset);
    }

    if (user.pin_tft_mosi != -1)
    {
        Serial.print("MOSI    = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_mosi));
    }
    if (user.pin_tft_miso != -1)
    {
        Serial.print("MISO    = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_miso));
    }
    if (user.pin_tft_clk != -1)
    {
        Serial.print("SCK     = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_clk));
    }

#ifdef ARDUINO_ARCH_ESP8266
    if (user.overlap == true)
    {
        Serial.println("Overlap selected, following pins MUST be used:");

        Serial.println("MOSI     = SD1 (GPIO 8)");
        Serial.println("MISO     = SD0 (GPIO 7)");
        Serial.println("SCK      = CLK (GPIO 6)");
        Serial.println("TFT_CS   = D3  (GPIO 0)\n");

        Serial.println("TFT_DC and TFT_RST pins can be user defined");
    }
#endif
    String pinNameRef = "GPIO ";
#ifdef ARDUINO_ARCH_ESP8266
    pinNameRef = "PIN_D";
#endif

    if (user.esp == 0x32F)
    {
        Serial.println("\n>>>>> Note: STM32 pin references above D15 may not reflect board markings <<<<<");
        pinNameRef = "D";
    }
    if (user.pin_tft_cs != -1)
    {
        Serial.print("TFT_CS   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_cs));
    }
    if (user.pin_tft_dc != -1)
    {
        Serial.print("TFT_DC   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_dc));
    }
    if (user.pin_tft_rst != -1)
    {
        Serial.print("TFT_RST  = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_rst));
    }

    if (user.pin_tch_cs != -1)
    {
        Serial.print("TOUCH_CS = " + pinNameRef);
        Serial.println(getPinName(user.pin_tch_cs));
    }

    if (user.pin_tft_wr != -1)
    {
        Serial.print("TFT_WR   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_wr));
    }
    if (user.pin_tft_rd != -1)
    {
        Serial.print("TFT_RD   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_rd));
    }

    if (user.pin_tft_d0 != -1)
    {
        Serial.print("\nTFT_D0   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d0));
    }
    if (user.pin_tft_d1 != -1)
    {
        Serial.print("TFT_D1   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d1));
    }
    if (user.pin_tft_d2 != -1)
    {
        Serial.print("TFT_D2   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d2));
    }
    if (user.pin_tft_d3 != -1)
    {
        Serial.print("TFT_D3   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d3));
    }
    if (user.pin_tft_d4 != -1)
    {
        Serial.print("TFT_D4   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d4));
    }
    if (user.pin_tft_d5 != -1)
    {
        Serial.print("TFT_D5   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d5));
    }
    if (user.pin_tft_d6 != -1)
    {
        Serial.print("TFT_D6   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d6));
    }
    if (user.pin_tft_d7 != -1)
    {
        Serial.print("TFT_D7   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d7));
    }

#if defined(TFT_BL)
    Serial.print("\nTFT_BL           = " + pinNameRef);
    Serial.println(getPinName(user.pin_tft_led));
#if defined(TFT_BACKLIGHT_ON)
    Serial.print("TFT_BACKLIGHT_ON = ");
    Serial.println(user.pin_tft_led_on == HIGH ? "HIGH" : "LOW");
#endif
#endif

    Serial.println();

    uint16_t fonts = tft.fontsLoaded();
    if (fonts & (1 << 1))
        Serial.print("Font GLCD   loaded\n");
    if (fonts & (1 << 2))
        Serial.print("Font 2      loaded\n");
    if (fonts & (1 << 4))
        Serial.print("Font 4      loaded\n");
    if (fonts & (1 << 6))
        Serial.print("Font 6      loaded\n");
    if (fonts & (1 << 7))
        Serial.print("Font 7      loaded\n");
    if (fonts & (1 << 9))
        Serial.print("Font 8N     loaded\n");
    else if (fonts & (1 << 8))
        Serial.print("Font 8      loaded\n");
    if (fonts & (1 << 15))
        Serial.print("Smooth font enabled\n");
    Serial.print("\n");

    if (user.serial == 1)
    {
        Serial.print("Display SPI frequency = ");
        Serial.println(user.tft_spi_freq / 10.0);
    }
    if (user.pin_tch_cs != -1)
    {
        Serial.print("Touch SPI frequency   = ");
        Serial.println(user.tch_spi_freq / 10.0);
    }

    Serial.println("[/code]");
}

void readWriteTest(TFT_eSPI tft)
{
    tft.fillScreen(0xF81F);
    static uint32_t wr = 1;
    static uint32_t rd = 0xFFFFFFFF;

    for (int i = 1; i < 16; i++)
    {
        delay(500);

        tft.drawPixel(30, 30, wr);
        Serial.print(" Pixel value written = ");
        Serial.println(wr, HEX);

        rd = tft.readPixel(30, 30);
        Serial.print(" Pixel value read    = ");
        Serial.println(rd, HEX);

        if (rd != wr)
        {
            Serial.println(" ERROR                 ^^^^");
            // while(1) yield();
        }
        else
            Serial.println(" PASS ");
        // Walking 1 test
        wr = wr << 1;
        if (wr >= 0x10000)
            wr = 1;
    }
}
