#include <Arduino.h>
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-StepperDriver/StepperDriver.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 500

#define STEP_EN 32
#define STEP_DIR 33

bool debouncing = false;
hw_timer_t *debounce_timer;
QueueHandle_t displayQueueHandle;
QueueHandle_t motorQueueHandle;

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
		xQueueSendFromISR(displayQueueHandle, &toggle, &toBack);
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

void StepperTask(void *params)
{
	QueueHandle_t *queue = (QueueHandle_t *)params;

	pinMode(STEP_EN, OUTPUT);
	pinMode(STEP_DIR, OUTPUT);

	digitalWrite(STEP_EN, 0);
	digitalWrite(STEP_DIR, 0);

	for (;;)
	{
		bool trash;
		if (xQueueReceive(*queue, &trash, 2) == pdTRUE)
		{
			digitalWrite(STEP_EN, digitalRead(STEP_EN) ^ 1);
		}
		delay(15);
	}
}

void setup(void)
{
	Serial.begin(115200);

	initDebounceTimer();

	displayQueueHandle = xQueueCreate(1, sizeof(int));
	motorQueueHandle = xQueueCreate(1, sizeof(int));

	BaseType_t dispSuccess = xTaskCreate(DisplayTask, "Display", 64000, &displayQueueHandle, 2, nullptr);
	BaseType_t stepSuccess = xTaskCreate(StepperTask, "Step", 32000, &motorQueueHandle, 1, nullptr);

	Serial.print("Display Task Status: ");
	Serial.println(dispSuccess);
	Serial.print("Step Task Status: ");
	Serial.println(stepSuccess);

	// Create Start Stop Button Interrupt.
	pinMode(17, INPUT_PULLUP); // Set pin 17 to pullup input.
	attachInterrupt(digitalPinToInterrupt(17), ToggleISR, RISING);
}

void loop()
{
}
