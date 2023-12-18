
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

/**
 * @brief Abstract Driver for Fluid Delivery Driver
 * 
 */
class FluidDeliveryDriver {
    virtual FluidDeliveryError setFlowRate (int freq) = 0;

    virtual void disable() = 0;
    virtual void enable() = 0;

    virtual int getDistanceFB() = 0;
};

class FluidDeliveryError {
    virtual int getID() = 0;
};



class FluidControlEvent
{
public:
    virtual int getID() = 0;
};

typedef struct {
    int switchVolume;
    int newRate;
} BolusSettings;


void ControlTask(void *params);

#endif