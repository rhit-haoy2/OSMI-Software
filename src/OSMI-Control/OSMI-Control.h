
#ifndef __OSMI_CONTROL_H
#define __OSMI_CONTROL_H
#include <FastPID.h>
#include <Arduino.h>

typedef struct ControlState
{
    FastPID pidChannels[4];    // Holds PID channels for the systems.
    bool activeChannels[4];    // Which channels are being controlled by the PID.
    int availableChannelCount; // How many channels are physically plugged into the system.

    // Keep units in mL / Hr & assume a conversion of 1 revolution == 1 ml for demo only.
    unsigned int dosageRates[4];
    // Need to keep track of target position.
    unsigned int targetPosition[4];
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