#include <Arduino.h>
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-Control/OSMI-Control.h"
#include "OSMI-WIFI/OSMI-WIFI.h"
#include "OSMI-SelfStart/SelfStart.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#include "driver/ledc.h"
#include "OSMI-Control/FluidDeliveryController.h"

#include "OSMI-SelfStart/SelfStart.h"

#define SPI_DRIVER_CS 27
#define MOTOR_PWM_PIN 26
#define LIMIT_SWITCH_PIN 25
#define DIST_PER_STEP 1.0

#define ESTOP_PIN 34
#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 1000

QueueHandle_t displayQueueHandle;
static bool debouncing = false;
static hw_timer_t* debounce_timer;

void setup(void)
{
	Serial.begin(115200);
	Serial.println("[92mOSMI Startup[0m");
	SPI.begin();

	// Create the communication lines between tasks. Usually only one number at a time.
	displayQueueHandle = xQueueCreate(1, sizeof(int));

	// Configure display struct.
	display_config_t displayConfig = {
		//.driver = driverInst,
		.handle = &displayQueueHandle,
	};

#ifdef SKIP_POST
	BaseType_t dispSuccess = xTaskCreate(DisplayTask, "DISP", 64000, &displayConfig, 2, nullptr);
#else
	BaseType_t postSuccess = xTaskCreate(SelfStartTestTask, "POST", 128, nullptr, configMAX_PRIORITIES, nullptr);
#endif
}

// Unused.
void loop() {}