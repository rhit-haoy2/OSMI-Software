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

        uint16_t positRaw = 0;

        // Feedback from potentiometer.
        int raw = 0;
        int Vin = 5;
        float Vout = 0;
        float R1 = 10000; // known resistance
        float R = 0;      // unknown resistance
        float buffer = 0;

        // TODO: Reconfigure to dosage rate. See chart for example.
        // position
        positRaw = analogRead(A0); // reads value of the potentiometer
        Serial.print("position: ");
        Serial.println(positRaw);

        // Find our new delay time from our setpoint.
        Serial.print("setpoint: ");
        Serial.println(positRaw);

        // Write to PWM
        analogWriteFrequency(positRaw);

        state->pulseFrequency = positRaw;
        analogWrite(STEP_EN, state->enabled ? MOTOR_ENABLED : MOTOR_DISABLED);

        delay(600); // delay for system tick
    }
}
