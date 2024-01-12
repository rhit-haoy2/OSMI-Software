#include "OSMI-Control.h"
#include "FluidDeliveryController.h"

#include <driver/pcnt.h>

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void ESP32PwmSpiDriver::initPWM(void)
{

    pinMode(stepPin, OUTPUT);
    analogWriteFrequency(DEFAULT_FREQUENCY);
    analogWrite(stepPin, 0);
}

static pcnt_isr_handle_t isrHandle = NULL;

static void IRAM_ATTR handlePCNTOverflow(void *arg)
{
    Serial.println("ESP32 PWM SPI Driver: implement PCNT Overflow.");
}

void initPulseCounter(int stepPin, ESP32PwmSpiDriver *driver)
{
    // Config for the unit.
    pcnt_config_t config = {
        .pulse_gpio_num = stepPin,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DIS,
        .counter_h_lim = (int16_t)0xffff - 1,
        .counter_l_lim = -1,
        .unit = DEFAULT_PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };

    pcnt_unit_config(&config);
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_isr_register(handlePCNTOverflow, &driver, 0, &isrHandle);
}

ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin)
{
    countUp = true;

    // Setup micro stepper
    microStepperDriver = new DRV8434S();
    microStepperDriver->setChipSelectPin(chipSelectPin);
    this->stepPin = stepPin;

    delay(1); // Allow for stepper to wake up.

    microStepperDriver->resetSettings();
    microStepperDriver->disableSPIStep(); // Ensure STEP pin is stepping.

    uint8_t ctrl5 = microStepperDriver->getCachedReg(DRV8434SRegAddr::CTRL5);
    ctrl5 = ctrl5 | 0x10;
    microStepperDriver->setReg(DRV8434SRegAddr::CTRL1, ctrl5); // enable stall detection

    uint8_t ctrl4 = microStepperDriver->getCachedReg(DRV8434SRegAddr::CTRL5);
    ctrl4 = ctrl4 | 0x10;
    microStepperDriver->setReg(DRV8434SRegAddr::CTRL1, ctrl4); // enable open load detection.
    // todo configure step mode.
    Serial.print("Settings applied: ");
    Serial.println(microStepperDriver->verifySettings());

    // Setup PWM
    this->initPWM();

    // TODO Setup Pulse Counter
    initPulseCounter(stepPin, this);
}

float ESP32PwmSpiDriver::getDistanceFB()
{
    Serial.println("IMPLEMENT ESP32::getFeedback!");
    int16_t pulses = 0;
    pcnt_get_counter_value(DEFAULT_PCNT_UNIT, &pulses);
    return float(pulses);
}

void ESP32PwmSpiDriver::enable()
{
    Serial.println("IMPLEMENT ESP32::enable!");
    // TODO: enable pulsecounter.
    pcnt_counter_resume(DEFAULT_PCNT_UNIT);

    // Re-enable driver.
    microStepperDriver->enableDriver(); // Enable the driver.
    Serial.print("Enabling Driver: ");
    Serial.println(microStepperDriver->verifySettings());

    // Enable PWM.
    analogWrite(stepPin, 128);
}

/// @brief Disable the driver.
void ESP32PwmSpiDriver::disable()
{
    Serial.println("IMPLEMENT ESP32::disable!");

    // disable PWM. Stop motor layer 1.
    analogWrite(stepPin, 0);

    // Disable motor driver. Stop motor layer 2.
    microStepperDriver->disableDriver();

    // Good to stop pulse counter.
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
}

bool ESP32PwmSpiDriver::occlusionDetected()
{
    // Measure torque
    uint16_t torque_low = microStepperDriver->driver.readReg(DRV8434SRegAddr::CTRL8);
    uint16_t torque_high = microStepperDriver->driver.readReg(DRV8434SRegAddr::CTRL9) & 0x0F;
    uint16_t torque = (torque_high << 8) + torque_low;

    // Get threshold if learnt.
    uint16_t thresh_low = microStepperDriver->driver.readReg(DRV8434SRegAddr::CTRL6);
    uint16_t thresh_high = microStepperDriver->driver.readReg(DRV8434SRegAddr::CTRL7) & 0x0F;
    uint16_t threshold = (torque_high << 8) + torque_low;

    return torque <= threshold; // Torque approaches zero as more greatly loaded.
}

FluidDeliveryError *ESP32PwmSpiDriver::checkFault() {
    uint8_t fault = microStepperDriver->readFault();

    FluidDeliveryError *faultWrapper = 0;

    if (fault != 0)
    {
        faultWrapper = new FluidDeliveryError(fault);
        Serial.print("Fault: ");
        Serial.println(fault, 2);
        return faultWrapper;
    }

    // Verify there is no occlusion.
    if (occlusionDetected())
    {
        faultWrapper = new FluidDeliveryError((uint8_t) DRV8434SFaultBit::STL);
        return faultWrapper;
    }

    faultWrapper = new FluidDeliveryOK();
    return faultWrapper;
}

/// @brief Set the frequency of flow rate into the system in ml/minute
/// @param freq Frequency at which mL is delivered.
/// @return The error that was encountered when setting the value.
FluidDeliveryError *ESP32PwmSpiDriver::setFlowRate(unsigned int freq)
{
    Serial.println("IMPLEMENT ESP32::setFlowRate!");

    analogWriteFrequency(freq);
    return checkFault();
}