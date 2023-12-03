#include "StepperDriver.h"

void StepperTask(void *params)
{
	/**Pin Setup*/
	pinMode(STEP_DIR, OUTPUT);
	pinMode(STEP_EN, ANALOG);
	analogWriteFrequency(DEFAULT_FREQUENCY);
	analogWrite(STEP_EN, MOTOR_DISABLED);
	/** End pin setup*/

	StepperParams_t paramStruct = *(StepperParams_t *)params;
	QueueHandle_t *toggleQueue = paramStruct.toggleQueue;
	QueueHandle_t *freqQueue = paramStruct.frequencyQueue;
	QueueHandle_t *displayQueue = paramStruct.displayQueue;

	/**Pre Scheduler Setep*/
	int analog_frequency = DEFAULT_FREQUENCY; // default 1000Hz
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
			xQueueSend(*displayQueue, &enabled, 2);
		}
		// END Toggle Logic.

		delay(30);
	}
}
