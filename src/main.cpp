#include <Arduino.h>
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-StepperDriver/StepperDriver.h"
#include "OSMI-Control/OSMI-Control.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#include "driver/ledc.h"

#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 1000

bool debouncing = false;
hw_timer_t *debounce_timer;
QueueHandle_t displayQueueHandle;
QueueHandle_t motorQueueHandle;
QueueHandle_t freqQueue;
QueueHandle_t ctrlQueue;
StepperParams_t stepperTaskParams;

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

	/** Pin Interrupt Setup*/
	initDebounceTimer();
	// Create Start Stop Button Interrupt.
	pinMode(17, INPUT_PULLUP); // Set pin 17 to pullup input.
	attachInterrupt(digitalPinToInterrupt(17), ToggleISR, RISING);

	// Create the communication lines between tasks. Usually only one number at a time.
	displayQueueHandle = xQueueCreate(1, sizeof(int));
	motorQueueHandle = xQueueCreate(1, sizeof(int));
	freqQueue = xQueueCreate(1, sizeof(int));
	ctrlQueue = xQueueCreate(1, sizeof(int));

	// Struct for toggle queue
	stepperTaskParams = {
		.toggleQueue = &motorQueueHandle,
		.frequencyQueue = &freqQueue,
		.displayQueue = &displayQueueHandle};

	ControlState *ctrlState = (ControlState *)malloc(sizeof(ControlState));
	SetupControl(ctrlState);
	ctrlState->messageQueue = &ctrlQueue;

	BaseType_t cntlSuccess = xTaskCreate(ControlTask, "CNTL", 64000, ctrlState, 2, nullptr);
	BaseType_t dispSuccess = xTaskCreate(DisplayTask, "DISP", 64000, &displayQueueHandle, 3, nullptr);
	BaseType_t stepSuccess = xTaskCreate(StepperTask, "STEP", 16000, &stepperTaskParams, 1, nullptr);

	Serial.print("Display Task Status: ");
	Serial.println(dispSuccess);
	Serial.print("Step Task Status: ");
	Serial.println(stepSuccess);
}

// Loop never reached.
void loop() {}
