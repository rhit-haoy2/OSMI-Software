
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <Arduino.h>
//#include <hal/ledc_hal.h>
#include <driver/ledc.h>

extern "C"
{
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
}

#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

typedef enum
{
    Depress,
    Reverse,
} direction_t;

void ControlWatchdogTask();

struct
{
    unsigned int switchVolume;
    unsigned int newRate;
} BolusSettings;

class FluidDeliveryController
{
public:
    virtual bool startFlow() = 0;
    virtual bool stopFlow() = 0;

    /// @brief Reverse the direction of the flow.
    virtual void reverse() = 0;

    /// @brief Set fluid flow rate in milliliter per minute
    /// @param flowRate flow rate in mL/min
    virtual void setFlow(double flowRateMlPerMin) = 0;

    /// @brief get the total volume delivered from the controller.
    /// @return the volume delivered in mL.
    virtual double getVolumeDelivered() = 0;

    /// @brief Get the current status of the controller whether it's delivering, moving start/stopping.
    /// @return
    virtual int getStatus() = 0;
};

/**
 * @brief Abstract Driver for Fluid Delivery Driver
 *
 */
class FluidDeliveryDriver
{

public:
    /// @brief Get the distance feedback from system.
    /// @return The distance in mm
    virtual double getDistanceMm() = 0;

    /// @brief Set the velocity of travel.
    /// @param mmPerMinute Speed at which to move.
    /// @return Success
    virtual int setVelocity(double mmPerMinute) = 0;
    virtual int getStatus() = 0;

    virtual void disable() = 0;
    virtual void enable() = 0;

    virtual void setDirection(direction_t direction) = 0;
    virtual direction_t getDirection(void) = 0;

    virtual bool occlusionDetected() = 0;
};

#endif