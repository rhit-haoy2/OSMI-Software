#include "FluidDeliveryController.h"
#include <driver/pcnt.h>
#include <driver/gpio.h>
#include <math.h>

#define STL_LRN_OK 0b10000
#define STALL 0b1000

static const char *TAG = "ESP32PwmSpiDriver";

static pcnt_config_t upConfig = {
    // Set PCNT input signal and control GPIOs
    .pulse_gpio_num = PCNT_PIN_NOT_USED,
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,

    .lctrl_mode = PCNT_MODE_KEEP, // Do not change value based on CTRL
    .hctrl_mode = PCNT_MODE_KEEP, // Do not change value based on CTRL

    // What to do on the positive / negative edge of pulse input?
    .pos_mode = PCNT_COUNT_INC, // Count up on the positive edge
    .neg_mode = PCNT_COUNT_DIS, // Keep the counter value on the negative edge

    // What to do when control input is low or high?
    // Set the maximum and minimum limit values to watch
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

    .pos_mode = PCNT_COUNT_DEC, // unlike above, decrement.
    .neg_mode = PCNT_COUNT_DIS,

    .counter_h_lim = 32767,
    .counter_l_lim = -32768,

    .unit = DEFAULT_PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
};

static ledc_channel_config_t stepPinChannelConfig = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0,
};

static ledc_timer_config_t stepPinTimerConfig = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_10_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 100,
    .clk_cfg = LEDC_AUTO_CLK,
};

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

/// @brief Initialize the pulse counter systems.
/// @param  void
void ESP32PwmSpiDriver::initPulseCounter(void)
{
    // Set pulse counter pin.
    upConfig.pulse_gpio_num = stepPin;
    downConfig.pulse_gpio_num = stepPin;

    pcnt_unit_config(&downConfig);

    // Enable overflow events.
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(DEFAULT_PCNT_UNIT, PCNT_EVT_L_LIM);

    // pause for isr configs.
    pcnt_counter_pause(DEFAULT_PCNT_UNIT);
    pcnt_filter_enable(DEFAULT_PCNT_UNIT);
    pcnt_set_filter_value(DEFAULT_PCNT_UNIT, 1);

    // install ISR service
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(DEFAULT_PCNT_UNIT, handlePCNTOverflow, (void *)this);

    // Clear for first use.
    pcnt_counter_clear(DEFAULT_PCNT_UNIT);
    pcnt_counter_resume(DEFAULT_PCNT_UNIT);
}

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void ESP32PwmSpiDriver::initPWM(void)
{
    // See: https://www.esp32.com/viewtopic.php?t=18115
    ledc_timer_config(&stepPinTimerConfig);

    stepPinChannelConfig.gpio_num = stepPin;

    Serial.print("Channel Config OK: ");
    Serial.println(ledc_channel_config(&stepPinChannelConfig) == ESP_OK ? "True" : "False");
}

void ESP32PwmSpiDriver::initStepperDriver(int chipSelectPin)
{
    // Setup micro stepper
    microStepperDriver = DRV8434S();
    microStepperDriver.setChipSelectPin(chipSelectPin);

    delay(5); // Allow for stepper to wake up.
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
    this->microStepSetting = 32;

    // todo configure step mode.
    Serial.print("Settings applied: ");
    Serial.println(microStepperDriver.verifySettings());
}

void ESP32PwmSpiDriver::initGPIO()
{
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add((gpio_num_t)stopPin, limitISRHandler, (void *)this);

    gpio_set_direction((gpio_num_t)stepPin, GPIO_MODE_INPUT_OUTPUT);
    gpio_matrix_in(stepPin, PCNT_SIG_CH0_IN1_IDX, 0);
    gpio_matrix_out(stepPin, LEDC_HS_SIG_OUT0_IDX + LEDC_CHANNEL_0, 1, 0);
}

/// @brief Constructor for ESP32 PWM Serial-Peripheral Interface Driver.
/// @param chipSelectPin Chip Select for DRV8434S
/// @param stepPin Step pin for DRV8434S
/// @param stopPin Limit switch pin.
/// @param pitch Distance per rotation of stepper motor in millimeters.
/// @param degreesPerStep degrees per step of the stepper motor.
ESP32PwmSpiDriver::ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int stopPin, double pitch, double degreesPerStep)
{
    this->stepPin = stepPin;
    this->distancePerRotMm = pitch;
    this->degreesPerStep = degreesPerStep;
    this->distanceSteps = 0; // Assumes at 0. Should run thru calibrate routine.
    this->chipSelectPin = chipSelectPin;

    io_conf = gpio_config_t{
        .pin_bit_mask = (1ull << stopPin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    initStepperDriver(chipSelectPin);
    mutex = xSemaphoreCreateMutex();
}

void ESP32PwmSpiDriver::initialize()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    initStepperDriver(chipSelectPin);

    // Setup PWM
    initPWM();

    // Setup Pulse Counter
    initPulseCounter();

    // initialize gpio. Do last for final mux settings.
    initGPIO();
    xSemaphoreGive(mutex);
}

/// @brief Get the distance (in mm) away from the endstop.
/// @return distance (in mm) away from end stop.
double ESP32PwmSpiDriver::getDistanceMm()
{
    int64_t microSteps = getDistanceSteps();

    // 92160 == 360 deg/rot * 256 uSteps / step
    double distance = microSteps * degreesPerStep / (92160.0F * distancePerRotMm);

    return distance;
}

/// @brief Get the distance in micro-steps.
/// @param  void
/// @return Distance in 256 microsteps / actual step.
int64_t ESP32PwmSpiDriver::getDistanceSteps(void)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    unsigned long globalTime = millis() - this->pulseTime;
    if (status == Moving)
    {
        int64_t longCount = globalTime * frequency / 1000;
        if (getDirection() == Depress)
        {
            longCount = 0 - longCount;
        }
        int64_t result = this->distanceSteps + (longCount * (256 / microStepSetting));
        this->distanceSteps = result;
    }
    this->pulseTime = millis();
    xSemaphoreGive(mutex);
    
    return this->distanceSteps;
}

/// @brief Turn off the driver.
void ESP32PwmSpiDriver::enable()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    // Re-enable driver.
    microStepperDriver.clearFaults(); // Clear faults on the driver.

    if (microStepperDriver.verifySettings())
    {
        Serial.println("Stepper Driver had to be updated.");
        microStepperDriver.applySettings();
    }

    // Enable PWM.
    stepPinChannelConfig.duty = 100;
    ledc_channel_config(&stepPinChannelConfig);

    this->pulseTime = millis();

    this->status = EspDriverStatus_t::Moving;

    // Enable Stall Detection learning.
    uint8_t ctrl5 = microStepperDriver.getCachedReg(DRV8434SRegAddr::CTRL5);
    microStepperDriver.setReg(DRV8434SRegAddr::CTRL5, ctrl5 ^ (1 << 6));
    xSemaphoreGive(mutex);
}

/// @brief Disable the driver.
void ESP32PwmSpiDriver::disable()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    // disable PWM. Stop motor layer 1.
    stepPinChannelConfig.duty = 0;
    ledc_channel_config(&stepPinChannelConfig);

    this->status = EspDriverStatus_t::Stopped;
    xSemaphoreGive(mutex);
}

/// @brief Set the direction of the motor.
/// @param direction A direction type, either depress or reverse.
void ESP32PwmSpiDriver::setDirection(direction_t direction)
{
    xSemaphoreTake(mutex, portMAX_DELAY);

    switch (direction)
    {
    case Reverse:
        // set the pulse counter direction the inverse of up / down.
        Serial.println("Reverse");
        pcnt_unit_config(&upConfig);
        microStepperDriver.setDirection(false); // todo confirm directioning or make reconfigurable.
        break;
    case Depress:
    default:
        // Change pulse counter direction to count up.
        Serial.println("Forward");
        pcnt_unit_config(&downConfig);
        microStepperDriver.setDirection(true);
        break;
    }

    xSemaphoreGive(mutex);
}

/// @brief Get the current direction from the driver.
/// @param  void
/// @return dirction_t direction either forward or reverse.
direction_t ESP32PwmSpiDriver::getDirection(void)
{
    return microStepperDriver.getDirection() ? Depress : Reverse;
}

/// @brief Try to detect an occlusion.
/// @return
bool ESP32PwmSpiDriver::occlusionDetected()
{
    // Measure torque
    uint16_t torque_low = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL8);
    uint16_t torque_high = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL9) & 0x0F;
    uint16_t torque = (torque_high << 8) + torque_low;

    // Get threshold if learnt.
    uint16_t thresh_low = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL6);
    uint16_t thresh_high = microStepperDriver.driver.readReg(DRV8434SRegAddr::CTRL7) & 0x0F;
    uint16_t threshold = (thresh_high << 8) + thresh_low;

    return torque <= threshold; // Torque approaches zero as more greatly loaded.
}

/// @brief Stop the counter and reset to zero.
void ESP32PwmSpiDriver::disableInIsr()
{
    this->disable();
    this->status = limitStopped;
    pcnt_counter_clear(DEFAULT_PCNT_UNIT);
    this->distanceSteps = 0;
}

/// @brief sets the current number of steps from an ISR. Do not call this function in userspace.
/// @param steps the current number of steps
void ESP32PwmSpiDriver::setStepsInIsr(int64_t steps)
{
    xSemaphoreTakeFromISR(mutex, nullptr);

    this->distanceSteps = steps;

    xSemaphoreGiveFromISR(mutex, nullptr);
}

/// @brief Set the velocity of the carriage in mm per minute.
/// @param mmPerMinute The velocity of the carriage.
/// @return Success == 0, error < 0
int ESP32PwmSpiDriver::setVelocity(double mmPerMinute)
{
    getDistanceSteps();
    xSemaphoreTake(mutex, portMAX_DELAY);

    if (mmPerMinute < 0)
    {
        return -1;
    }

    double distancePerStepMm = distancePerRotMm * degreesPerStep / 360.0F;

    double stepPerSecond = mmPerMinute / distancePerStepMm;

    if (stepPerSecond >= 14000)
    {
        return -1;
    }

    // full winding step / second.
    microStepperDriver.setStepMode(DRV8434SStepMode::MicroStep1);
    microStepSetting = 1;
    while (stepPerSecond <= 50)
    {
        stepPerSecond = stepPerSecond * 2;
        microStepSetting = microStepSetting * 2;
        if (microStepSetting > 256)
        {
            disable();
            return -1;
        }
    }
    stepPerSecond = mmPerMinute * microStepSetting / distancePerStepMm;

    uint32_t herz = round(stepPerSecond);

    if (herz <= 0)
    {
        ESP_LOGE(TAG, "Step-Hz less than zero: %.1f%%", stepPerSecond);
        return -1;
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

    stepPinTimerConfig.freq_hz = herz;
    esp_err_t timerOk = ledc_timer_config(&stepPinTimerConfig);
    this->pulseTime = millis();
    this->frequency = herz;
    xSemaphoreGive(mutex);

    return 0;
}

int ESP32PwmSpiDriver::getStatus(void)
{
    return this->status;
}