#include "OSMI-Control.h"

int setChannelStatus(bool newStatus, int channelHandle, ControlState *state)
{
    // guard against invalid channelHandle
    if (channelHandle != 0)
    {
        return 1;
    }

    return 0;
}

int SetupControl(ControlState *state)
{
    // control system init
    float K = 100; // tuning parameter

    FastPID channel1 = FastPID(K, 0, 0, 66, 32, false);
    state->pidChannel = channel1;
    state->startTime = 0;
    state->enabled = false;

    return 0; // Return with no Errors.
}

void ControlTask(void *params)
{
    ControlState *state = (ControlState *)params;
    SetupControl(state); // Initialize Control Parameter.
    while (1)
    {
        int positRaw = 5000;
        // Find our new delay time from our setpoint.
        Serial.print("setpoint: ");
        Serial.println(positRaw);

        // Write to PWM
        analogWriteFrequency(1000);

        state->pulseFrequency = positRaw;
        analogWrite(STEP_EN, state->enabled ? MOTOR_ENABLED : MOTOR_DISABLED);

        delay(600); // delay for system tick
    }
}
