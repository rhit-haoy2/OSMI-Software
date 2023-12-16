#include <Arduino.h>

class FluidControlEvent
{
public:
    virtual int getID() = 0;
};

class FluidDeliveryController
{
public:
    virtual bool startFlow() = 0;
    virtual bool stopFlow() = 0;

    virtual void handleDispatch(FluidControlEvent *e);
    virtual QueueHandle_t getQueue() = 0;

    /// @brief get the total volume delivered from the controller.
    /// @return the volume delivered in mL.
    virtual float getVolumeDelivered() = 0;

    FluidDeliveryController(){};
    virtual ~FluidDeliveryController(){};

protected:
    QueueHandle_t queue;
};

class StartFlowEvent : FluidControlEvent
{
public:
    int getID();
};

class StopFlowEvent : FluidControlEvent
{
public:
    int getID();
};