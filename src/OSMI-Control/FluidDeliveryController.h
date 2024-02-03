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

} EspDriverStatus_t;

/// @brief ESP32 Instance of a Driver.
class ESP32PwmSpiDriver : public FluidDeliveryDriver
{
public:
    ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int stopPin, float pitch, float degreesPerStep);

    float getDistanceMm(void);
    float getDistanceSteps(void);

    int setVelocity(float mmPerMinute);
    int getStatus(void);

    void disable();
    void enable(void);

    void setDirection(direction_t direction);
    direction_t getDirection(void);
    /// @brief Set the current direction as counting up.
    void setCountUpDirection(direction_t direction);
    bool occlusionDetected(void);

    void disableInIsr();
    void setStepsInIsr(unsigned long long steps);

    ///@brief do not use under normal circumstances.
    void resetFeedback(void);

private:
    int stepPin;
    int stopPin;
    /// @brief Step is full winding step.
    float distancePerRotMm;
    float degreesPerStep;
    unsigned long long distanceSteps;
    EspDriverStatus_t status;
    int inverseDirection = 0;
    DRV8434S microStepperDriver;
    void initPWM(void);
    void initPulseCounter(void);
};

/// @brief ECE Senior Design Team 11 (2023-2024) Implementation of Control Scheme
class Team11Control : public FluidDeliveryController
{
public:
    Team11Control(float mlPerMm, FluidDeliveryDriver *driverInstance);
    ~Team11Control();

    /// @brief  Start the flow
    /// @param  void
    /// @return Success.
    bool startFlow(void);

    /// @brief Stop the flow.
    /// @param  void
    /// @return Success.
    bool stopFlow(void);

    /// @brief Changes the direction of the flow rate. Does not work unless flow is stopped.
    /// @param  void
    void reverse(void);

    void setFlow(float flowRateMlPerMin);
    void setVolumetricConversion(float mlPerMm);

    /// @brief Get Volume in milliliters delivered.
    /// @param  void
    /// @return mL delivered as of now.
    float getVolumeDelivered(void);

    /// @brief Get the current status of the control system.
    /// @param  void
    /// @return Current state of the system.
    int getStatus(void) { return state; };

    int configureDosage(float bolusRate, float bolusVolume, float infusionRate, float infusionVolume);

    // Gabe, Describe your function here.
    void controlTaskUpdate(void);

    FluidDeliveryDriver *getDriver() { return this->driver; };

private:
    // Task Handle here.

    FastPID p_Controller;
    FluidDeliveryDriver *driver;
    TaskHandle_t controlTask;
    float volumePerDistance;
    float bolusRate = -1;
    float bolusVolume = -1;
    float infusionRate = -1;
    float infusionVolume = -1;
    float prevBolusVolume = 0;
    float currBolusVolume = 0;
    unsigned long startTime = 0;
    int state;
};

#endif