#include "FluidDeliveryController.h"

/// @brief Gets the error ID for StopFlowEvent.
/// @return
int StopFlowEvent::getID()
{
    return -1;
}

int StartFlowEvent::getID()
{
    return 1;
}

int CheckSystemEvent::getID()
{
    return 0;
}

