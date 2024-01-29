#include "OSMI-Control.h"
#include <driver/timer.h>
#include <freertos/queue.h>
#include "FluidDeliveryController.h"
#include <FastPID.h>
#include <DRV8434S.h>

#include <hal/ledc_hal.h>
#include <driver/ledc.h>

#define GROUP timer_group_t::TIMER_GROUP_0
#define TIMER timer_idx_t::TIMER_1

#define TIMER_DIVIDER 8
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)

/*Implementation for ControlTask */

void Team11ControlTask(void *parameters)
{
    Team11Control *controlSystem = (Team11Control *)parameters;

    while (1)
    {
        controlSystem->controlTaskUpdate();
        delay(100);
    }
}

Team11Control::Team11Control(float volumePerDistance, FluidDeliveryDriver *driver)
{
    // TODO setup p_controller with appropriate values.
    this->p_Controller = FastPID(0, 0, 0, 1);
    this->driver = driver;
    this->volumePerDistance = volumePerDistance;

    // Create handle here with unique ID reference
    char identifier = (char)65 + ((unsigned int)this) % 29; // create id hash.
    String id = "11Ctrl_";                                  // default string
    char rep[2] = {identifier, 0};                          // create char* for replacing "_"
    id.replace("_", rep);                                   // replace "_"

    // create task here with unique handle
    TaskHandle_t handle = 0;
    xTaskCreate(Team11ControlTask, id.c_str(), 1000, this, 3, &handle); // returns success or fail. if fail should handle, but not now
    this->controlTask = handle;
}

void Team11Control::controlTaskUpdate()
{
    float setpoint;
    float feedback = getVolumeDelivered();
    unsigned long currTime = millis() - startTime; // f*** the user timer
    // TODO: check if currTime is less than startTime

    switch (state)
    {
    case 2: // infusion delivery
        setpoint = (infusionRate * currTime) + bolusVolume;
        break;
    case 1: // bolus delivery
        setpoint = bolusRate * currTime;
        break;

    case 0: // stop
    default:
        return;
    }

    unsigned long long newSpeed = this->p_Controller.step(lroundf(setpoint), lroundf(feedback));
    this->driver->setVelocity(newSpeed);
}

Team11Control::~Team11Control()
{
    // Delete task here.
    vTaskDelete(this->controlTask);
}

bool Team11Control::startFlow()
{
    driver->enable();
    startTime = millis();
    return true;
}

bool Team11Control::stopFlow()
{
    driver->disable();
    return true;
}

void Team11Control::reverse(void)
{
    direction_t direction = this->driver->getDirection();
    if (direction == Depress)
    {
        direction = Reverse;
    }
    else
    {
        direction = Depress;
    }

    this->driver->setDirection(direction);
}

void Team11Control::setFlow(float flowRateMlPerMin)
{
    float velocity = flowRateMlPerMin / volumePerDistance;
    this->driver->setVelocity(velocity);
}

float Team11Control::getVolumeDelivered()
{
    float distance = this->driver->getDistanceMm();
    float volume = volumePerDistance * distance;
    return volume;
}

int Team11Control::configureDosage(float bolusRate, float bolusVolume, float infusionRate, float infusionVolume)
{
    if (bolusRate <= 0 || bolusVolume < 0 || infusionRate <= 0 || infusionVolume <= 0)
    {
        return -1;
    }

    this->bolusRate = bolusRate;
    this->bolusVolume = bolusVolume;
    this->infusionRate = infusionRate;
    this->infusionVolume = infusionVolume;

    return 0;
}
