#include <Arduino.h>
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-Control/OSMI-Control.h"
#include "OSMI-WIFI/OSMI-WIFI.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#include "driver/ledc.h"
#include "OSMI-Control/FluidDeliveryController.h"

#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 1000

#define SPI_DRIVER_CS 27
#define MOTOR_PWM_PIN 26

bool debouncing = false;
hw_timer_t *debounce_timer;
QueueHandle_t displayQueueHandle;
QueueHandle_t motorQueueHandle;
QueueHandle_t ctrlQueue;

/* Toggle Button Setup */

void ToggleISR()
{
	if (!debouncing)
	{
		timerRestart(debounce_timer);
		timerStart(debounce_timer);
		debouncing = true;

		BaseType_t toBack = pdTRUE;
		bool toggle = true;
		xQueueSendFromISR(motorQueueHandle, &toggle, &toBack);
	}
}

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

/* End Toggle Button */

void setup(void)
{
	Serial.begin(115200);
	Serial.println("[42mOSMI Startup[0m");
	SPI.begin();

	/** Pin Interrupt Setup*/
	initDebounceTimer();

	// Create Start Stop Button Interrupt.
	pinMode(17, INPUT_PULLUP); // Set pin 17 to pullup input.
	attachInterrupt(digitalPinToInterrupt(17), ToggleISR, RISING);

	// Create the communication lines between tasks. Usually only one number at a time.
	displayQueueHandle = xQueueCreate(1, sizeof(int));
	motorQueueHandle = xQueueCreate(1, sizeof(int));

	FluidDeliveryDriver *driverInst = (FluidDeliveryDriver *)new ESP32PwmSpiDriver(SPI_DRIVER_CS, MOTOR_PWM_PIN); // init driver with SPI CS & PWM motor pin.
	ControlState *controlState = new ControlState(1, driverInst); // Setup control with the driver instance and queue.

	// Configure display struct.
	display_config_t displayConfig = {
		.controller = controlState,
		.handle = &displayQueueHandle,
	};

	BaseType_t cntlSuccess = xTaskCreate(ControlTask, "CNTL", 16000, controlState, 3, nullptr);
	BaseType_t dispSuccess = xTaskCreate(DisplayTask, "DISP", 64000, &displayConfig, 2, nullptr);
	// BaseType_t wifiSUccess = xTaskCreate(WIFI_Task, "WIFI", 8000, nullptr, 1, nullptr);
}

// Unused.
void loop() {}