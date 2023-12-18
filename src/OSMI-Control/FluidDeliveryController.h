#include <Arduino.h>
#include "OSMI-Control.h"
#include <DRV8434S.h>

class FluidDeliveryController
{
public:
    virtual void handleDispatch(FluidControlEvent *e);
    virtual QueueHandle_t getQueue() = 0;

    virtual bool startFlow() = 0;
    virtual bool stopFlow() = 0;

    /// @brief get the total volume delivered from the controller.
    /// @return the volume delivered in mL.
    virtual float getVolumeDelivered() = 0;

    FluidDeliveryController(){};
    virtual ~FluidDeliveryController(){};

protected:
    QueueHandle_t queue;
    float volumeDeliveredCache;
};

class CheckSystemEvent : FluidControlEvent
{
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

class SetDosageEvent : FluidControlEvent
{
public:
    SetDosageEvent(BolusSettings settings) { this->settings = settings; }
    int getID()
    {
        return 4;
    };
    BolusSettings getSettings()
    {
        return this->settings;
    }

private:
    BolusSettings settings;
};

/// @brief ESP32 Instance of a Driver.
class ESP32PwmSpiDriver : FluidDeliveryDriver {
    public:
        ESP32PwmSpiDriver(int chipSelectPin, int stepPin);
        FluidDeliveryError* setFlowRate (int freq);
        void disable();
        void enable();

        float getDistanceFB();
    private:
        int stepPin;
        DRV8434S microStepperDriver;

};

/// @brief ECE Senior Design Team 11 (2023-2024) Implementation of Control Scheme 
class ControlState : public FluidDeliveryController
{
public:
    ControlState(QueueHandle_t queue, float volumePerDistance, FluidDeliveryDriver* driverInstance);
    QueueHandle_t getQueue();
    void handleDispatch(FluidControlEvent *e);

    float getVolumeDelivered();

    bool startFlow();
    bool stopFlow();

private:
    // GABE Describe your function here.

    FastPID p_Controller;
    BolusSettings settings;
    FluidDeliveryDriver* driver;
};
