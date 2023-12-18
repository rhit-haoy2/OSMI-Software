#include "OSMI-Control.h"
#include "FluidDeliveryController.h"

ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin) {

    //Setup micro stepper
    this->microStepperDriver = DRV8434S();
    this->microStepperDriver.setChipSelectPin(chipSelectPin);

    microStepperDriver.resetSettings();
    microStepperDriver.disableSPIStep(); // Ensure STEP pin is stepping.

    this->stepPin = stepPin;

    //Setup PWM

    //Setup Pulse Counter
    
}

float ESP32PwmSpiDriver::getDistanceFB() {
    Serial.println("IMPLEMENT ESP32::getFeedback!");
    return infinityf();
}

void ESP32PwmSpiDriver::enable() {
    Serial.println("IMPLEMENT ESP32::enable!");
    microStepperDriver.enableDriver(); // Enable the driver.
    //TODO: enable pulsecounter.
    //TODO: Enable PWM.
}
void ESP32PwmSpiDriver::disable() {
    Serial.println("IMPLEMENT ESP32::disable!");
    //TODO: disable PWM.
    
    // Disable motor driver.
    microStepperDriver.disableDriver();

    //TODO: disable pulse counter so no erroneous flow counts.


}


/// @brief Set the frequency of flow rate into the system in ml/minute
/// @param freq Frequency at which mL is delivered.
/// @return The error that was encountered when setting the value.
FluidDeliveryError* ESP32PwmSpiDriver::setFlowRate (int freq) {
    Serial.println("IMPLEMENT ESP32::setFlowRate!");

    uint8_t fault = microStepperDriver.readFault();

    FluidDeliveryError* faultWrapper = new FluidDeliveryOK();

    if (fault != 0) {
        *faultWrapper = FluidDeliveryError(fault);
    }

    return faultWrapper;
    
}