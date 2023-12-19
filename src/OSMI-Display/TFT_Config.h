#include <Arduino.h>
#include <TFT_eSPI.h>

void printProcessorName(void);
int8_t getPinName(int8_t pin);
void readSetup(TFT_eSPI tft);
void readWriteTest(TFT_eSPI tft);