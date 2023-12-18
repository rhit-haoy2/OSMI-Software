
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

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