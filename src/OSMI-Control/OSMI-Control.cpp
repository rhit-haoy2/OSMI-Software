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

void ControlTask(void *params)
{
    FluidDeliveryController *state = (FluidDeliveryController *)params;

    /**TODO Setup timer for sending an update fluid status*/
    

    while (1)
    {
        FluidControlEvent *e;

        xQueueReceive(state->getQueue(), e, portMAX_DELAY);
        state->handleDispatch(e);
    }
}

ControlState::ControlState(QueueHandle_t queue, float volumePerDistance)
{
    this->queue = queue;
    this->p_Controller = FastPID(volumePerDistance, 0, 0, 2);
}

QueueHandle_t ControlState::getQueue()
{
    return this->queue;
}

bool ControlState::startFlow()
{
    return xQueueSend(this->queue, new StartFlowEvent(), portMAX_DELAY);
}

bool ControlState::stopFlow()
{
    return xQueueSend(this->queue, new StopFlowEvent(), portMAX_DELAY);
}

void ControlState::handleDispatch(FluidControlEvent *event)
{
    Serial.print("Dispatching Event: ");
    Serial.println(event->getID());
}

float ControlState::getVolumeDelivered()
{
    // TODO: Implement
    return 0;
}
