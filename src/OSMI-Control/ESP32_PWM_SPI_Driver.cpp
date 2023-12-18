#include "FluidDeliveryController.h"


ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin) {
    this->csPin = chipSelectPin;
    
}

float ESP32PwmSpiDriver::getDistanceFB() {
    Serial.println("IMPLEMENT ESP32::getFeedback!");
    return nanf();
}

void ESP32PwmSpiDriver::enable() {
    Serial.println("IMPLEMENT ESP32::enable!");
}
void ESP32PwmSpiDriver::disable() {
    Serial.println("IMPLEMENT ESP32::disable!");
}

FluidDeliveryError ESP32PwmSpiDriver::setFlowRate (int freq) {
    Serial.println("IMPLEMENT ESP32::setFlowRate!");
}