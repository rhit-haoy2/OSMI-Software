#include "FluidDeliveryController.h"
#include <driver/pcnt.h>
#include <driver/gpio.h>
#include <math.h>

static const char *TAG = "ESP32PwmSpiDriver";
static pcnt_config_t upConfig = {
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DIS,
    .counter_h_lim = (int16_t)0xffff - 1,
    .counter_l_lim = -1,
    .unit = DEFAULT_PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
};

static pcnt_config_t downConfig = {
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,
    .pos_mode = PCNT_COUNT_DEC,
    .neg_mode = PCNT_COUNT_DIS,
    .counter_h_lim = (int16_t)0xffff - 1,
    .counter_l_lim = -1,
    .unit = DEFAULT_PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
};

// Allows for static inversion of counting up or down.
static pcnt_config_t *configs[2] = {&upConfig, &downConfig};

static pcnt_isr_handle_t isrHandle = 0;

static void IRAM_ATTR handlePCNTOverflow(void *arg)
{
    Serial.println("ESP32 PWM SPI Driver: implement PCNT Overflow.");
    ESP32PwmSpiDriver *driver = (ESP32PwmSpiDriver *)arg;
    int16_t pcnt_steps = 0;
    pcnt_get_counter_value(DEFAULT_PCNT_UNIT, &pcnt_steps);
    if (pcnt_steps <= 0)
    {
        driver->setStepsInIsr(driver->getDistanceSteps() - 0xffff);
    }
    else
    {
        driver->setStepsInIsr(driver->getDistanceSteps() + 0xffff);
    }
}

static void IRAM_ATTR limitISRHandler(void *driverInst)
{
    ESP32PwmSpiDriver *driver = (ESP32PwmSpiDriver *)driverInst;
    driver->disableInIsr();
}

void ESP32PwmSpiDriver::initPulseCounter(void)
{
    // Config for the unit.
    upConfig.pulse_gpio_num = this->stepPin;
    downConfig.pulse_gpio_num = this->stepPin;
    pcnt_unit_config(&upConfig);
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_L_LIM);
    pcnt_isr_register(handlePCNTOverflow, this, 0, &isrHandle);
}

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void ESP32PwmSpiDriver::initPWM(void)
{
    pinMode(stepPin, ANALOG);
    analogWrite(stepPin, 0);
    analogWriteFrequency(1000);
}

ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int stopPin, float pitch, float degreesPerStep)
{

    // Setup micro stepper
    microStepperDriver = DRV8434S();
    microStepperDriver.setChipSelectPin(chipSelectPin);

    this->stepPin = stepPin;
    this->distancePerRotMm = pitch;
    this->degreesPerStep = degreesPerStep;

    delay(5); // Allow for stepper to wake up.

    // Setup PWM
    this->initPWM();
    microStepperDriver.resetSettings();
    microStepperDriver.disableSPIStep(); // Ensure STEP pin is stepping.

    uint8_t ctrl5 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL5);
    ctrl5 = ctrl5 | 0x10;
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL5, ctrl5); // enable stall detection

    uint8_t ctrl4 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL4);
    ctrl4 = ctrl4 | 0x10;
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL4, ctrl4); // enable open load detection.

    microStepperDriver.setCurrentMilliamps(1000);

    // todo configure step mode.
    Serial.print("Settings applied: ");
    Serial.println(microStepperDriver.verifySettings());

    // esp32 gpio interrupt setup
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ull << stopPin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add((gpio_num_t)stopPin, limitISRHandler, (void *)this);

    // Setup Pulse Counter
    initPulseCounter();
}

float ESP32PwmSpiDriver::getDistanceMm()
{
    int16_t pulses = 0;
    pcnt_get_counter_value(DEFAULT_PCNT_UNIT, &pulses);
    unsigned long long steps = this->distanceSteps + pulses;
    float distance = steps / distancePerRotMm;

    return distance;
}

float ESP32PwmSpiDriver::getDistanceSteps(void)
{
    // TODO Factor in the microstep divider.
    return this->distanceSteps;
}

void ESP32PwmSpiDriver::enable()
{

    pcnt_counter_resume(DEFAULT_PCNT_UNIT);

    // Re-enable driver.
    microStepperDriver.clearFaults();  // Clear faults on the driver.
    microStepperDriver.enableDriver(); // Enable the driver.
    Serial.print("Enabling Driver: ");
    Serial.println(microStepperDriver.verifySettings());

    // Enable PWM.
    analogWrite(stepPin, 128);
    this->status = EspDriverStatus_t::Bolus;
}

/// @brief Disable the driver.
void ESP32PwmSpiDriver::disable()
{
    // disable PWM. Stop motor layer 1.
    analogWrite(stepPin, 0);

    // Disable motor driver. Stop motor layer 2.
    microStepperDriver.disableDriver();

    // Good to stop pulse counter.
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    this->status = EspDriverStatus_t::Stopped;
}

void ESP32PwmSpiDriver::setDirection(direction_t direction)
{
    // Guard against changing directions while moving.
    if (status == Bolus)
        return;

    switch (direction)
    {
    Reverse:
        // set the pulse counter direction the inverse of up / down.
        pcnt_unit_config(configs[inverseDirection ^ 1]);
        microStepperDriver.setDirection(false); // todo confirm directioning or make reconfigurable.
        break;
    Depress:
    default:
        // Change pulse counter direction to count up.
        pcnt_unit_config(configs[inverseDirection]);
        microStepperDriver.setDirection(true);
    }
}

direction_t ESP32PwmSpiDriver::getDirection(void)
{
    return microStepperDriver.getDirection() ? Depress : Reverse;
}

void ESP32PwmSpiDriver::setCountUpDirection(direction_t direction)
{
    switch (direction)
    {
    case Reverse:
        inverseDirection = 1;
        break;

    default:
        inverseDirection = 0;
        break;
    }
}

bool ESP32PwmSpiDriver::occlusionDetected()
{
    // Measure torque
    uint16_t torque_low = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL8);
    uint16_t torque_high = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL9) & 0x0F;
    uint16_t torque = (torque_high << 8) + torque_low;
    Serial.print("TRQ ");
    Serial.println(torque);

    // Get threshold if learnt.
    uint16_t thresh_low = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL6);
    uint16_t thresh_high = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL7) & 0x0F;
    uint16_t threshold = (thresh_high << 8) + thresh_low;
    Serial.print("Thresh: ");
    Serial.println(threshold);

    return torque <= threshold; // Torque approaches zero as more greatly loaded.
}

void ESP32PwmSpiDriver::disableInIsr()
{
    this->disable();
    this->status = limitStopped;
}

/// @brief sets the current number of steps from an ISR. Do not call this function in userspace.
/// @param steps the current number of steps
void ESP32PwmSpiDriver::setStepsInIsr(unsigned long long steps)
{
    this->distanceSteps = steps;
}

/// @brief  Do not call under normal circumstances. See set steps in ISR.
/// @param  void
void ESP32PwmSpiDriver::resetFeedback(void)
{
    setStepsInIsr(0);
}

int ESP32PwmSpiDriver::setVelocity(float mmPerMinute)
{
    Serial.print("Setting Velocity ");
    Serial.print(mmPerMinute);
    Serial.println(" mm/min");
    if (mmPerMinute < 0)
    {
        return -1;
    }

    float distancePerStepMm = distancePerRotMm * degreesPerStep / 360.0F;


    // full winding step / second.
    float stepPerSecond = mmPerMinute * 60.0F / distancePerStepMm;
    int micro_phase = 0;

    for (int i = 0; i < 7; i++)
    {
        if (stepPerSecond >= 2000.0F)
            break;

        if (micro_phase <= 0)
        {
            // double the step rate..
            stepPerSecond = stepPerSecond * 2;
            micro_phase = 3;
        }
        else if (micro_phase >= 0b1010)
        {
            micro_phase = 0b1010;
        }
        else
        {
            // double the step rate..
            stepPerSecond = stepPerSecond * 2;
            micro_phase++;
        }
    }

    Serial.print("Step Per Second ");
    Serial.println(stepPerSecond, 3);
    uint32_t herz = round(stepPerSecond);

    if (herz <= 0)
    {
        ESP_LOGE(TAG, "Step-Hz less than zero: %.1f%%", stepPerSecond);
        return -1;
    }

    Serial.print("Stephz ");
    Serial.println(herz);
    Serial.print("microphase ");
    Serial.println(micro_phase);

    analogWriteFrequency(herz);
    this->microStepperDriver.setStepMode(micro_phase);
    return 0;
}

int ESP32PwmSpiDriver::getStatus(void)
{
    Serial.println("IMPLEMENT ESP32::getStatus!");
    return this->status;
}