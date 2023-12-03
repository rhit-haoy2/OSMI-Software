#ifndef _STEPPER_DRIVER_H_
#define _STEPPER_DRIVER_H_

#include <Arduino.h>

typedef struct StepperParams
{
    QueueHandle_t *toggleQueue;
    QueueHandle_t *frequencyQueue;
    QueueHandle_t *displayQueue;
} StepperParams_t;

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

void StepperTask(void *params);

#endif