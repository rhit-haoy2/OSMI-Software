#include "OSMI-Control.h"
#include <driver/timer.h>
#include <freertos/queue.h>
#include "FluidDeliveryController.h"

#include <DRV8434S.h>

#include <hal/ledc_hal.h>
#include <driver/ledc.h>

#define GROUP timer_group_t::TIMER_GROUP_0
#define TIMER timer_idx_t::TIMER_1

#define TIMER_DIVIDER 8
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)

/*Implementation for ControlTask */

Team11Control::Team11Control(float volumePerDistance, FluidDeliveryDriver *driver)
{
    this->p_Controller = FastPID(volumePerDistance, 0, 0, 2);
    this->driver = driver;
    this->volumePerDistance = volumePerDistance;
}

Team11Control::~Team11Control()
{
    // Delete task here.
}

bool Team11Control::startFlow()
{
    driver->enable();
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
    if(direction == Depress) {
        direction = Reverse;
    } else {
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
    // TODO: Implement
    return 0;
}