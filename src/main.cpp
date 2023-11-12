#include <Arduino.h>
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-StepperDriver/StepperDriver.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#include "driver/ledc.h"

#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 1000

#define DEFAULT_FREQUENCY 100
#define MOTOR_ENABLED 128
#define MOTOR_DISABLED 0

#define STEP_EN 32
#define STEP_DIR 33

typedef struct StepperParams
{
	QueueHandle_t *toggleQueue;
	QueueHandle_t *frequencyQueue;
} StepperParams_t;

bool debouncing = false;
hw_timer_t *debounce_timer;
QueueHandle_t displayQueueHandle;
QueueHandle_t motorQueueHandle;
QueueHandle_t freqQueue;
StepperParams_t t;

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

void StepperTask(void *params)
{
	StepperParams_t paramStruct = *(StepperParams_t *)params;
	QueueHandle_t *toggleQueue = paramStruct.toggleQueue;
	QueueHandle_t *freqQueue = paramStruct.frequencyQueue;

	/**Pre Scheduler Setep*/
	int analog_frequency = DEFAULT_FREQUENCY; //default 1000Hz
	pinMode(STEP_DIR, OUTPUT);
	pinMode(STEP_EN, ANALOG);

	analogWrite(STEP_EN, 0);
	analogWriteFrequency(analog_frequency);

	digitalWrite(STEP_DIR, 0);
	bool enabled = false;

	for (;;)
	{
		int tempFrequency = 100;
		if (xQueueReceive(*freqQueue, &tempFrequency, 2) == pdTRUE)
		{
			analog_frequency = tempFrequency;
			Serial.print("New Frequency: ");
			Serial.println(analog_frequency);
		}

		// TOGGLE LOGIC
		bool trash;
		if (xQueueReceive(*toggleQueue, &trash, 2) == pdTRUE)
		{
			Serial.println("Step Received Toggle");
			enabled = !enabled;
			analogWriteFrequency(analog_frequency);
			analogWrite(STEP_EN, enabled ? MOTOR_ENABLED : MOTOR_DISABLED);
			xQueueSend(displayQueueHandle, &enabled, 2);
		}
		// END Toggle Logic.

		delay(30);
	}
}

void setup(void)
{
	Serial.begin(115200);

	initDebounceTimer();

	displayQueueHandle = xQueueCreate(1, sizeof(int));

	motorQueueHandle = xQueueCreate(1, sizeof(int));
	freqQueue = xQueueCreate(1, sizeof(int));
	t = {
		.toggleQueue = &motorQueueHandle,
		.frequencyQueue = &freqQueue
	};

	/**Pre-Scheduler Motor Setup*/
	pinMode(STEP_DIR, OUTPUT);
	pinMode(STEP_EN, ANALOG);

	analogWriteFrequency(DEFAULT_FREQUENCY);
	analogWrite(STEP_EN, MOTOR_DISABLED); 
	/** End Pre-schedulermotor setup*/

	BaseType_t dispSuccess = xTaskCreate(DisplayTask, "Display", 64000, &displayQueueHandle, 2, nullptr);
	BaseType_t stepSuccess = xTaskCreate(StepperTask, "Step", 32000, &t, 1, nullptr);

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
