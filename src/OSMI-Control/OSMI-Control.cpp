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
    this->state = 0;

    // Create handle here with unique ID reference
    char identifier = (char)65 + ((unsigned int)this) % 29; // create id hash.
    String id = "11Ctrl_";                                  // default string
    char rep[2] = {identifier, 0};                          // create char* for replacing "_"
    id.replace("_", rep);                                   // replace "_"

    // create task here with unique handle
    TaskHandle_t handle = 0;
    xTaskCreate(Team11ControlTask, id.c_str(), 8000, this, 3, &handle); // returns success or fail. if fail should handle, but not now
    this->controlTask = handle;
}

void Team11Control::controlTaskUpdate()
{

    float setpoint;
    float feedback = getVolumeDelivered();
    unsigned long currTime = millis() - startTime; // f*** the user timer

    bool detected = driver->occlusionDetected();
    //bool detected = false;
    if (detected)
    {
        this->state = 4;
        Serial.println("Occlusion detected.");
    }

    // Switch State
    switch (this->state)
    {
    case 3: // Temporary stop infusion state.
        state = 0;
        break;
    case 2:                                           // infusion volume comparison.
        state = (feedback >= infusionVolume) ? 3 : 2; // if feedback >= infusion max volume, go to stop state.
        break;
    case 1: // bolus delivery
        state = (feedback >= bolusVolume) ? 2 : 1;
        break;
    case 0:
    default:
        break;
    }

    // Action Based on State
    switch (state)
    {
    case 3: // temporary stop state.
        this->driver->disable();
        Serial.println("Infusion Completed");
        this->state = 0;
        return;
    case 2: // infusion delivery
        setpoint = (infusionRate * currTime) + bolusVolume;
        break;
    case 1: // bolus delivery
        setpoint = bolusRate * currTime;
        break;

    case 0:
    default:
        return;
    }

    // Set velocity for cases 2 & 1.

    // unsigned long long newSpeed = this->p_Controller.step(lroundf(setpoint), lroundf(feedback));
    // Serial.print("Control Task New Speed ");
    // Serial.println(newSpeed);
    // this->driver->setVelocity(newSpeed);
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
    this->state = 1;
    return true;
}

bool Team11Control::stopFlow()
{
    this->state = 3;
    driver->disable();
    float feedback = driver->getDistanceMm();
    Serial.print("Moved: ");
    Serial.print(feedback);
    Serial.println(" mm");
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
    if (bolusRate < 0 || bolusVolume < 0 || infusionRate <= 0 || infusionVolume <= 0)
    {
        return -1;
    }

    this->bolusRate = bolusRate;
    this->bolusVolume = bolusVolume;
    this->infusionRate = infusionRate;
    this->infusionVolume = infusionVolume;

    return 0;
}
