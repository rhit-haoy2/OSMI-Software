
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>

#define STEP_EN 32
#define STEP_DIR 33

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

typedef struct ControlState
{
    FastPID pidChannel; // Holds PID channels for the systems.

    // Keep units in mL / Hr & assume a conversion of 1 revolution == 1 ml for demo only.
    unsigned int pulseFrequency;
    unsigned long startTime;

    bool enabled;

    QueueHandle_t *messageQueue;
} ControlState;

/**
 * @brief Disables all active channels.
 *
 * @return int Error State. Non-zero == Error.
 */
int stopAllChannels();

/**
 * @brief Set the
 *
 * @param newStatus True == enabled.
 * @param channelHandle Which channel to be enabled
 * @param state System State
 * @return int Whether successfully set new state.
 */
int setChannelStatus(bool newStatus, int channelHandle, ControlState *state);

/**Task functions, decides where each stepper needs to be at a given point.*/
int SetupControl(ControlState *control);

void ControlTask(void *params);

#endif