
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>
#include "FluidDeliveryController.h"

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

class ControlState : public FluidDeliveryController
{
public:
    ControlState(QueueHandle_t queue, float volumePerDistance);
    QueueHandle_t getQueue();
    void handleDispatch(FluidControlEvent *e);

    float getVolumeDelivered();

    bool startFlow();
    bool stopFlow();

private:
    FastPID p_Controller;
};

void ControlTask(void *params);

#endif