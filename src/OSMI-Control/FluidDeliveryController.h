#include <Arduino.h>
#include "OSMI-Control.h"


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

class CheckSystemEvent : FluidControlEvent {
    public:
        int getID();        
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

class SetDosageEvent :FluidControlEvent {
    public:
    SetDosageEvent(BolusSettings settings) {this->settings = settings;}
    int getID() {
        return 4;
    };
    BolusSettings getSettings() {
        return this->settings;
    }

    private:
    BolusSettings settings;
    

};

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
    BolusSettings settings;
};
