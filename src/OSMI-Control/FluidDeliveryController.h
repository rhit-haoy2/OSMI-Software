#ifndef _FLUID_DELIVERY_CONTROLLER_H_
#define  _FLUID_DELIVERY_CONTROLLER_H_

#include <Arduino.h>
#include "OSMI-Control.h"
#include <DRV8434S.h>

#define PWM_TIMER LEDC_TIMER_2
#define PWM_SPEED ledc_mode_t::LEDC_HIGH_SPEED_MODE
#define PWM_CHANNEL LEDC_CHANNEL_4

class FluidDeliveryController
{
public:
    virtual QueueHandle_t getQueue() = 0;

    virtual bool startFlow() = 0;
    virtual bool stopFlow() = 0;

    
    /// @brief Set fluid tick rate
    /// @param flowRate flow rate in ml/min
    virtual void setFlow(unsigned int flowRate);

    /// @brief get the total volume delivered from the controller.
    /// @return the volume delivered in mL.
    virtual float getVolumeDelivered() = 0;

    FluidDeliveryController(){};
    virtual ~FluidDeliveryController(){};

    virtual void handleDispatch(FluidControlEvent *e);
protected:
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
class ESP32PwmSpiDriver : public FluidDeliveryDriver {
    public:
        ESP32PwmSpiDriver(int chipSelectPin, int stepPin);
        FluidDeliveryError* setFlowRate (unsigned int freq);
        void disable();
        void enable();

        float getDistanceFB();
    private:
        int stepPin;
        DRV8434S* microStepperDriver;
        void initPWM(void);

};

/// @brief ECE Senior Design Team 11 (2023-2024) Implementation of Control Scheme 
class ControlState : public FluidDeliveryController
{
public:
    ControlState(float volumePerDistance, FluidDeliveryDriver* driverInstance);
    QueueHandle_t getQueue();


    float getVolumeDelivered();
    void setFlow(unsigned int flowRate);

    bool startFlow();
    bool stopFlow();

    void handleDispatch(FluidControlEvent *e);
private:
    // GABE Describe your function here.

    FastPID p_Controller;
    BolusSettings settings;
    FluidDeliveryDriver* driver;
    QueueHandle_t queue;
    float absolutePosition;
};

#endif