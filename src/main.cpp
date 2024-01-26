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

/* End Toggle Button */

void IRAM_ATTR DebounceToggleISR()
{
	timerStop(debounce_timer);
	debouncing = false;
}

void initDebounceTimer()
{
	debounce_timer = timerBegin(DEBOUNCE_TIMER_ID, DEBOUNCE_PRESCALE, true);
	timerStop(debounce_timer);
	timerAttachInterrupt(debounce_timer, &DebounceToggleISR, true);
	timerAlarmWrite(debounce_timer, DEBOUNCE_THRESHOLD, true);
	timerAlarmEnable(debounce_timer);
}

bool debouncing = false;
hw_timer_t *debounce_timer;
/* ESTOP Setup */
static void IRAM_ATTR ESTOP_ISR()
{
	if (!debouncing)
	{
		timerRestart(debounce_timer);
		timerStart(debounce_timer);
		debouncing = true;

		BaseType_t toBack = pdTRUE;
		bool toggle = true;
		xQueueSendFromISR(displayQueueHandle, &toggle, &toBack);
	}
}

void setup(void)
{
	Serial.begin(115200);
	Serial.println("[92mOSMI Startup[0m");
	SPI.begin();

	/** Pin Interrupt Setup*/
	initDebounceTimer();

	// Create ESTOP Interrupt.
	pinMode(ESTOP_PIN, INPUT_PULLUP); // Set pin ESTOP_PIN to pullup input.
	attachInterrupt(digitalPinToInterrupt(ESTOP_PIN), ESTOP_ISR, RISING);

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