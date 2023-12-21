#include "OSMI-Control.h"
#include "FluidDeliveryController.h"

#include "hal/pcnt_hal.h"
#include "driver/pcnt.h"

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void ESP32PwmSpiDriver::initPWM(void)
{
    const ledc_timer_config_t timerConfig = {
        .speed_mode = PWM_SPEED,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = PWM_TIMER,
        .freq_hz = DEFAULT_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    const ledc_channel_config_t chanConfig = {
        .gpio_num = STEPPER_STEP,
        .speed_mode = PWM_SPEED,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));
    ESP_ERROR_CHECK(ledc_channel_config(&chanConfig));
}

void initPulseCounter(void) {

    // pcnt_unit_config_t pcntConfig = {
    //     .low_limit = -1,
    //     .high_limit = INT32_MAX,
    //     .intr_priority = 0
    // }

}

ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin)
{

    // Setup micro stepper
    microStepperDriver = new DRV8434S();
    microStepperDriver->setChipSelectPin(chipSelectPin);
    this->stepPin = stepPin;

    delay(1); // Allow for stepper to wake up.

    microStepperDriver->resetSettings();
    microStepperDriver->disableSPIStep(); // Ensure STEP pin is stepping.

    // Setup PWM
    this->initPWM();

    //TODO Setup Pulse Counter
}

float ESP32PwmSpiDriver::getDistanceFB()
{
    Serial.println("IMPLEMENT ESP32::getFeedback!");
    return infinityf();
}

void ESP32PwmSpiDriver::enable()
{
    Serial.println("IMPLEMENT ESP32::enable!");
    // TODO: enable pulsecounter.
    
    // Re-enable driver.
    microStepperDriver->enableDriver(); // Enable the driver.

    // Enable PWM.
    ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
    
}

/// @brief Disable the driver.
void ESP32PwmSpiDriver::disable()
{
    Serial.println("IMPLEMENT ESP32::disable!");

    //disable PWM. Stop motor layer 1.
    ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL)); 

    // Disable motor driver. Stop motor layer 2.
    microStepperDriver->disableDriver();

    // Good to stop pulse counter.
    // TODO: disable pulse counter so no erroneous flow counts.
}

/// @brief Set the frequency of flow rate into the system in ml/minute
/// @param freq Frequency at which mL is delivered.
/// @return The error that was encountered when setting the value.
FluidDeliveryError *ESP32PwmSpiDriver::setFlowRate(unsigned int freq)
{
    Serial.println("IMPLEMENT ESP32::setFlowRate!");

    uint8_t fault = this->microStepperDriver->readFault();


    ESP_ERROR_CHECK(ledc_set_freq(PWM_SPEED, PWM_TIMER, freq));

    FluidDeliveryError *faultWrapper = new FluidDeliveryOK();

    if (fault != 0)
    {
        *faultWrapper = FluidDeliveryError(fault);
    }

    return faultWrapper;
}