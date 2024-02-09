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

#define CONTROL_FREQ 2

#define TIMER_DIVIDER 8
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)

double currentvelocity;

/*Implementation for ControlTask */

void Team11ControlTask(void *parameters)
{
    Team11Control *controlSystem = (Team11Control *)parameters;

    while (1)
    {
        controlSystem->controlTaskUpdate();
        delay(1000 / CONTROL_FREQ);
    }
}

Team11Control::Team11Control(double volumePerDistance, FluidDeliveryDriver *driver)
{
    // TODO setup p_controller with appropriate values.
    this->kP = 0.1;
    this->kI = 0 * CONTROL_FREQ;
    this->kD = 0 * CONTROL_FREQ;

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

    double setpoint_mm_per_min;
    // global start time (no bolus)
    unsigned long curr_time_ms = millis() - startTime; // f*** the user timer

    // time after bolus.
    if ((state == 2) && (bolusVolume > 0))
    {
        curr_time_ms - (bolusVolume * 1000 / bolusRate); // calculate current time is in ms.
    }

    double feedback_mm = startPosition - this->driver->getDistanceMm();
    double feedback_mm_per_ms = (feedback_mm) * 1000 / curr_time_ms;

    bool detected = driver->occlusionDetected();

    // Switch State
    switch (this->state)
    {
    case 4:
    case 3: // Temporary stop infusion state.
        state = 0;
        break;
    case 2:                                                                    // infusion volume comparison.
        state = ((feedback_mm * volumePerDistance) >= infusionVolume) ? 3 : 2; // if feedback >= infusion max volume, go to stop state.
        break;
    case 1: // bolus delivery
        state = ((feedback_mm * volumePerDistance) >= bolusVolume) ? 2 : 1;
        break;
    case 0:
    default:
        break;
    }

    if (detected && (curr_time_ms > 2000) && (state > 0))
    {
        this->state = 4;
    }

    // Action Based on State
    switch (state)
    {
    case 4:
        this->driver->disable();
        return;
    case 3: // temporary stop state.
        this->driver->disable();
        this->state = 0;
        return;
    case 2: // infusion delivery
        setpoint_mm_per_min = infusionRate;
        break;
    case 1: // bolus delivery
        setpoint_mm_per_min = bolusRate;
        break;

    case 0:
    default:
        return;
    }

    // Set velocity for cases 2 & 1.

    double err = setpoint_mm_per_min - (feedback_mm_per_ms / 60000.0F);

    double new_speed_mm_per_min = (setpoint_mm_per_min + (err * kP) + (err * kI) + (err * kD));

    if (new_speed_mm_per_min > 80)
    {
        new_speed_mm_per_min = 80;
    }

    if (currentvelocity != new_speed_mm_per_min)
    {
        currentvelocity = new_speed_mm_per_min;
        this->driver->setVelocity(new_speed_mm_per_min);
    }
}

Team11Control::~Team11Control()
{
    // Delete task here.
    vTaskDelete(this->controlTask);
}

bool Team11Control::startFlow()
{
    driver->enable();
    startPosition = driver->getDistanceMm();
    startTime = millis();
    this->state = 1;
    return true;
}

bool Team11Control::stopFlow()
{
    this->state = 3;
    driver->disable();
    double feedback = driver->getDistanceMm();
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

void Team11Control::setFlow(double flowRateMlPerMin)
{
    double velocity = flowRateMlPerMin / volumePerDistance;
    this->driver->setVelocity(velocity);
}

double Team11Control::getVolumeDelivered()
{
    double distance = this->driver->getDistanceMm();
    double volume = volumePerDistance * distance;
    return volume;
}

int Team11Control::configureDosage(double bolusRate, double bolusVolume, double infusionRate, double infusionVolume)
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
