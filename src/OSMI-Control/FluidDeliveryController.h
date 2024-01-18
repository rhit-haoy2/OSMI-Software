#ifndef _FLUID_DELIVERY_CONTROLLER_H_
#define _FLUID_DELIVERY_CONTROLLER_H_

#include <Arduino.h>
#include "OSMI-Control.h"
#include <esp_log.h>
#include <DRV8434S.h>

#define PWM_TIMER LEDC_TIMER_2
#define PWM_SPEED ledc_mode_t::LEDC_HIGH_SPEED_MODE
#define PWM_CHANNEL LEDC_CHANNEL_4
#define DEFAULT_PCNT_UNIT PCNT_UNIT_0

typedef enum
{
    limitStopped = -1,
    Stopped = 0,
    Moving = 1,
    Reversed = 2,

} EspDriverStatus_t;

/// @brief ESP32 Instance of a Driver.
class ESP32PwmSpiDriver : public FluidDeliveryDriver
{
public:
    ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int stopPin, float distancePerStepMm);

    float getDistanceMm(void);
    float getDistanceSteps(void);

    int setVelocity(float mmPerMinute);
    int getStatus(void);

    void disable();
    void enable(void);

    void setDirection(direction_t direction);
    direction_t getDirection(void);
    bool occlusionDetected(void);

    void disableInIsr();
    void setStepsInIsr(unsigned long long steps);

private:
    int stepPin;
    int stopPin;
    float distancePerStepMm;
    unsigned long long distanceSteps;
    EspDriverStatus_t status;
    direction_t direction;
    DRV8434S microStepperDriver;
    void initPWM(void);
    void initPulseCounter(void);
};

/// @brief ECE Senior Design Team 11 (2023-2024) Implementation of Control Scheme
class Team11Control : public FluidDeliveryController
{
public:
    Team11Control(float volumePerDistance, FluidDeliveryDriver *driverInstance);
    ~Team11Control();

    bool startFlow(void);
    bool stopFlow(void);

    void reverse(void);

    void setFlow(float flowRateMlPerMin);
    void setVolumetricConversion(float mlPerMm);

    float getVolumeDelivered(void);
    int getStatus(void) { return state; };

    // Gabe, Describe your function here.


private:
    // Task Handle here.    

    FastPID p_Controller;
    FluidDeliveryDriver *driver;
    float volumePerDistance;
    int state;
};

#endif