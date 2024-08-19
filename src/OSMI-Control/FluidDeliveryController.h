#ifndef _FLUID_DELIVERY_CONTROLLER_H_
#define _FLUID_DELIVERY_CONTROLLER_H_

#include <Arduino.h>
#include "OSMI-Control.h"
#include <esp_log.h>
#include <DRV8434S.h>

#define DEFAULT_PCNT_UNIT PCNT_UNIT_1

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
    ESP32PwmSpiDriver(int chipSelectPin, int stepPin, int depressDirLimitPin, int retractDirLimitPin, double pitch, double degreesPerStep);

    double getDistanceMm(void);
    long long getDistanceSteps(void);

    int setVelocity(double mmPerMinute);
    int getStatus(void);

    void disable();
    void enable(void);

    int getStopPin(void);

    void setDirection(direction_t direction);
    direction_t getDirection(void);

    bool occlusionDetected(void);

    void disableInIsr();
    void setStepsInIsr(long long steps);

    void initialize(void);
    
    int stepPin;
    
    int depressDirLimitPin;
    int retractDirLimitPin;

private:
    
    int chipSelectPin;

    SemaphoreHandle_t mutex;

    /// @brief Step is full winding step.
    double distancePerRotMm;
    double degreesPerStep;
    unsigned int microStepSetting;
    unsigned long pulseTime;
    int frequency;

    gpio_config_t io_conf;

    // Distance in maximum microsteps (256).
    int64_t distanceSteps;

    EspDriverStatus_t status;
    int inverseDirection = 0;
    DRV8434S microStepperDriver;

    void initPWM(void);
    void initPulseCounter(void);
    void initStepperDriver(int chipSelectPin);
    void initGPIO();
};

/// @brief ECE Senior Design Team 11 (2023-2024) Implementation of Control Scheme
class Team11Control : public FluidDeliveryController
{
public:
    Team11Control(double mlPerMm, FluidDeliveryDriver *driverInstance);
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

    void setFlow(double flowRateMlPerMin);

    /// @brief Get Volume in milliliters delivered.
    /// @param  void
    /// @return mL delivered as of now.
    double getVolumeDelivered(void);

    /// @brief Get the current status of the control system.
    /// @param  void
    /// @return Current state of the system.
    int getStatus(void) { return state; };

    int configureDosage(double bolusRate, double bolusVolume, double infusionRate, double infusionVolume);

    // Gabe, Describe your function here.
    void controlTaskUpdate(void);

    FluidDeliveryDriver *getDriver() { return this->driver; };

private:
    // Task Handle here.

    FluidDeliveryDriver *driver;
    TaskHandle_t controlTask;
    double volumePerDistance;

    double kP;
    double kI;
    double kD;

    double bolusRate = -1;
    double bolusVolume = -1;
    double infusionRate = -1;
    double infusionVolume = -1;
    double prevBolusVolume = 0;
    double currBolusVolume = 0;

    unsigned long startTime = 0;
    unsigned long startPosition = 0;
    int state;
    QueueHandle_t startQueue;
};

#endif