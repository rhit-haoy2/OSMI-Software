
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>
#include <hal/ledc_hal.h>
#include <driver/ledc.h>

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

class FluidDeliveryError
{
public:
    FluidDeliveryError(int fault)
    {
        this->fault = fault;
    };

    int getID()
    {
        return fault;
    };

protected:
    int fault;
};

/***/
class FluidDeliveryOK : public FluidDeliveryError
{
public:
    FluidDeliveryOK() : FluidDeliveryError(0){};
};

class FluidControlEvent
{
public:
    virtual int getID() = 0;
};

typedef struct
{
    unsigned int switchVolume;
    unsigned int newRate;
} BolusSettings;

/**
 * @brief Abstract Driver for Fluid Delivery Driver
 *
 */
class FluidDeliveryDriver
{

public:
    virtual FluidDeliveryError *setFlowRate(unsigned int freq) = 0;

    virtual void disable() = 0;
    virtual void enable() = 0;

    /// @brief Get the distance feedback from system.
    /// @return The distance in mL
    virtual float getDistanceFB() = 0;
};

void ControlTask(void *params);

#endif