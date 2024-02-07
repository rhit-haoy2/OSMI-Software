#include "FluidDeliveryController.h"
#include <driver/pcnt.h>
#include <driver/gpio.h>
#include <math.h>
#define STL_LRN_OK 0b10000
#define STALL 0b1000


static const char *TAG = "ESP32PwmSpiDriver";
static pcnt_config_t upConfig = {
    .pulse_gpio_num = PCNT_PIN_NOT_USED,
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,

    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_KEEP,

    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DIS,

    .counter_h_lim = 32767,
    .counter_l_lim = -32768,

    .unit = DEFAULT_PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
};

static pcnt_config_t downConfig = {
    .pulse_gpio_num = PCNT_PIN_NOT_USED,
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,

    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_KEEP,

    .pos_mode = PCNT_COUNT_DEC,
    .neg_mode = PCNT_COUNT_DIS,

    .counter_h_lim = 32767,
    .counter_l_lim = -32768,

    .unit = DEFAULT_PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
};

ledc_channel_config_t stepPinChannelConfig = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0,
};

ledc_timer_config_t stepPinTimerConfig = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_10_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 100,
    .clk_cfg = LEDC_AUTO_CLK,
};

// Allows for static inversion of counting up or down.
static pcnt_config_t *configs[2] = {&upConfig, &downConfig};

static pcnt_isr_handle_t isrHandle = 0;

static void IRAM_ATTR handlePCNTOverflow(void *arg)
{
    ESP32PwmSpiDriver *driver = (ESP32PwmSpiDriver *)arg;

    uint32_t pcnt_event;
    pcnt_get_event_status(DEFAULT_PCNT_UNIT, &pcnt_event);
    if (pcnt_event == PCNT_EVT_L_LIM)
    {
        driver->setStepsInIsr(driver->getDistanceSteps() - 32768);
    }
    else if (pcnt_event == PCNT_EVT_H_LIM)
    {
        driver->setStepsInIsr(driver->getDistanceSteps() + 32767);
    }
}

static void IRAM_ATTR limitISRHandler(void *driverInst)
{
    ESP32PwmSpiDriver *driver = (ESP32PwmSpiDriver *)driverInst;
    driver->disableInIsr();
}

void ESP32PwmSpiDriver::initPulseCounter(void)
{
    // Set pulse counter pin.
    upConfig.pulse_gpio_num = this->stepPin;
    downConfig.pulse_gpio_num = this->stepPin;

    Serial.print("PCNT Configured: ");
    Serial.println(pcnt_unit_config(&upConfig) == ESP_OK);

    pcnt_filter_disable(DEFAULT_PCNT_UNIT);

    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_L_LIM);

    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    pcnt_counter_clear(DEFAULT_PCNT_UNIT);

    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(DEFAULT_PCNT_UNIT, handlePCNTOverflow, (void *)this);

    pcnt_intr_enable(DEFAULT_PCNT_UNIT);
}

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void ESP32PwmSpiDriver::initPWM(void)
{
    // See: https://www.esp32.com/viewtopic.php?t=18115
    Serial.print("Timer Config OK: ");
    Serial.println(ledc_timer_config(&stepPinTimerConfig) == ESP_OK ? "True" : "False");

    stepPinChannelConfig.gpio_num = stepPin;

    Serial.print("Channel Config OK: ");
    Serial.println(ledc_channel_config(&stepPinChannelConfig) == ESP_OK ? "True" : "False");
}

ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int stopPin, float pitch, float degreesPerStep)
{

    // Setup micro stepper
    microStepperDriver = DRV8434S();
    microStepperDriver.setChipSelectPin(chipSelectPin);

    this->stepPin = stepPin;
    this->distancePerRotMm = pitch;
    this->degreesPerStep = degreesPerStep;
    this->distanceSteps = 10240000; // set to 0 when done.
    this->microStepSetting = 1;

    delay(5); // Allow for stepper to wake up.

    // Setup PWM
    this->initPWM();

    // Setup Pulse Counter
    initPulseCounter();

    // Enable both devices.
    gpio_set_direction((gpio_num_t)stepPin, GPIO_MODE_INPUT_OUTPUT);
    gpio_matrix_out(stepPin, LEDC_HS_SIG_OUT0_IDX + LEDC_CHANNEL_0, 0, 0);

    // Microstepper Driver.
    microStepperDriver.resetSettings();
    microStepperDriver.disableSPIStep(); // Ensure STEP pin is stepping.
    microStepperDriver.enableSPIDirection();

    uint8_t ctrl5 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL5);
    ctrl5 = ctrl5 | 0x10;
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL5, ctrl5); // enable stall detection

    

    uint8_t ctrl4 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL4);
    ctrl4 = ctrl4 | 0x10;
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL4, ctrl4); // enable open load detection.

    microStepperDriver.setCurrentMilliamps(1000);
    microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep32);

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
}

/// @brief Get the distance (in mm) away from the endstop.
/// @return distance (in mm) away from end stop.
float ESP32PwmSpiDriver::getDistanceMm()
{
    unsigned long long steps = getDistanceSteps();
    // Steps to Rotation: 0.9 (deg / step) / 360 (deg / rot) / 256 uSteps / step
    float distance = steps * 0.000009765625F / distancePerRotMm;

    return distance;
}

/// @brief Get the distance in micro-steps.
/// @param  void
/// @return Distance in 256 microsteps / actual step.
unsigned long long ESP32PwmSpiDriver::getDistanceSteps(void)
{
    int16_t distance = 69;
    pcnt_get_counter_value(DEFAULT_PCNT_UNIT, &distance);
    return this->distanceSteps + (distance * 256 / microStepSetting);
}

void ESP32PwmSpiDriver::enable()
{

    Serial.print("Pulse counter resumed: ");
    Serial.println(pcnt_counter_resume(DEFAULT_PCNT_UNIT) == ESP_OK ? "True" : "False");

    // Re-enable driver.
    microStepperDriver.clearFaults();  // Clear faults on the driver.
    microStepperDriver.enableDriver(); // Enable the driver.
    Serial.print("Driver Ok: ");
    bool ok = microStepperDriver.verifySettings();
    if (!ok)
    {
        microStepperDriver.applySettings();
    }
    Serial.println(microStepperDriver.verifySettings());

    // Enable PWM.
    stepPinChannelConfig.duty = 100;
    Serial.print("PWM Enable OK: ");
    Serial.println(ledc_channel_config(&stepPinChannelConfig) == ESP_OK ? "True" : "False");

    this->status = EspDriverStatus_t::Moving;

    // set stall threshold to 0 while learning.
    //microStepperDriver.driver.writeReg(DRV8434SRegAddr::CTRL6, 0);
    //microStepperDriver.driver.writeReg(DRV8434SRegAddr::CTRL7, 0);

    // Enable Stall Detection learning.
    uint8_t ctrl5 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL5);
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL5, ctrl5 ^ (1 << 6));
}

/// @brief Disable the driver.
void ESP32PwmSpiDriver::disable()
{
    // disable PWM. Stop motor layer 1.
    stepPinChannelConfig.duty = 0;
    Serial.print("PWM disable OK: ");
    Serial.println(ledc_channel_config(&stepPinChannelConfig) == ESP_OK ? "True" : "False");

    // Disable motor driver. Stop motor layer 2.
    microStepperDriver.disableDriver();

    // Good to stop pulse counter.
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    this->status = EspDriverStatus_t::Stopped;
}

void ESP32PwmSpiDriver::setDirection(direction_t direction)
{
    // Guard against changing directions while moving.
    if (status == Moving)
    {
        Serial.println("Cannot change direction while moving.");
        return;
    }

    switch (direction)
    {
    case Reverse:
        // set the pulse counter direction the inverse of up / down.
        pcnt_unit_config(configs[inverseDirection ^ 1]);
        Serial.println("Reverse");
        microStepperDriver.setDirection(false); // todo confirm directioning or make reconfigurable.
        break;
    case Depress:
    default:
        // Change pulse counter direction to count up.
        Serial.println("Forward");
        pcnt_unit_config(configs[inverseDirection]);
        microStepperDriver.setDirection(true);
        break;
    }

    microStepperDriver.applySettings();
    microStepperDriver.verifySettings();
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
    uint8_t diag2 = microStepperDriver.readDiag2();
    String lrn_success = (diag2 & STL_LRN_OK) > 0 ? "True" : "False";
    String stall = (diag2 & STALL) > 0 ? "True" : "False";
    Serial.print("learning done ");
    Serial.println(lrn_success);
    Serial.print("stall ");
    Serial.println(stall);

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
    pcnt_counter_clear(DEFAULT_PCNT_UNIT);
    this->distanceSteps = 0;
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

    // Serial.print("Setting Velocity ");
    // Serial.print(mmPerMinute);
    // Serial.println(" mm/min");

    if (mmPerMinute < 0)
    {
        return -1;
    }

    // Serial.print("Direction: ");
    // Serial.println(microStepperDriver.getDirection() ? "True" : "False");
    float distancePerStepMm = distancePerRotMm * degreesPerStep / 360.0F;

    // full winding step / second.
    microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep1);
    float stepPerSecond = mmPerMinute / distancePerStepMm;
    microStepSetting = 1;

    if (stepPerSecond >= 14000)
    {
        Serial.printf("Velocity Too high!Too Fast!");
        Serial.printf("Stepshigh: %f", stepPerSecond, 3);
        return -1;
    }

    while (stepPerSecond <= 50)
    {
        stepPerSecond = stepPerSecond * 2;
        microStepSetting = microStepSetting * 2;
        if (microStepSetting > 256)
        {
            Serial.printf("Velocity Too low! Too Slow!");
            Serial.printf(" %f Hz\n", stepPerSecond, 3);
            return -1;
        }
    }

    switch (microStepSetting)
    {
    case 1:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep1);
        break;
    case 2:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep2);
        break;
    case 4:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep4);
        break;
    case 8:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep8);
        break;
    case 16:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep16);
        break;
    case 32:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep32);
        break;
    case 64:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep64);
        break;
    case 128:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep128);
        break;
    case 256:
        microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep256);
        break;
    default:
        break;
    }

    stepPerSecond = mmPerMinute * microStepSetting / distancePerStepMm;

    Serial.printf("Microstep: %d\n", microStepSetting);
    uint32_t herz = round(stepPerSecond);

    Serial.print("Step Per Second ");
    Serial.println(herz, 3);

    if (herz <= 0)
    {
        ESP_LOGE(TAG, "Step-Hz less than zero: %.1f%%", stepPerSecond);
        return -1;
    }

    stepPinTimerConfig.freq_hz = herz;
    esp_err_t timerOk = ledc_timer_config(&stepPinTimerConfig);

    Serial.print("Timer Config OK: ");
    Serial.println(timerOk == ESP_OK ? "True" : "False");

    return 0;
}

int ESP32PwmSpiDriver::getStatus(void)
{
    Serial.println("IMPLEMENT ESP32::getStatus!");
    return this->status;
}