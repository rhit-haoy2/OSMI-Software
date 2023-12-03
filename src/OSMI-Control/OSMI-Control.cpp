#include "OSMI-Control.h"

int setChannelStatus(bool newStatus, int channelHandle, ControlState *state)
{
    // guard against invalid channelHandle
    if (channelHandle > 3 || channelHandle < 0)
    {
        return 1;
    }

    state->activeChannels[channelHandle] = newStatus;
    return 0;
}

int SetupControl(ControlState* state)
{
    // control system init
    float K = 100; // tuning parameter

    FastPID channel1 = FastPID(K, 0, 0, 66, 32, false);
    state->pidChannels[0] = channel1;

    return 0; // Return with no Errors.
}

void ControlTask(void *params)
{
    ControlState *state = (ControlState *)params;
    SetupControl(state); // Initialize Control Parameter.
    for (;;)
    {

        uint16_t positRaw = 0;

        // resistance init
        int raw = 0;
        int Vin = 5;
        float Vout = 0;
        float R1 = 10000; // known resistance
        float R = 0;      // unknown resistance
        float buffer = 0;

        // position
        positRaw = analogRead(A0); // reads value of the potentiometer
        Serial.print("position: ");
        Serial.println(positRaw);

        // Find our new delay time from our setpoint.
        Serial.print("setpoint: ");
        Serial.println(positRaw);
        uint16_t new_delay = state->pidChannels[0].step(state->targetPosition[0], positRaw);

        if (state->activeChannels[0])
        {
            // TODO: set Wei's driver to new delay for current channel
            // kp < 1
            xQueueSend(*(state->messageQueue), &new_delay, 1);
        }

        delay(15); // delay for system tick
    }
}
