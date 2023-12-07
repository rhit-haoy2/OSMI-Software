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
	pinMode(STEP_DIR, OUTPUT); // Forward / Reverse
	pinMode(STEP_EN, ANALOG); //ANALOG == PWM

	analogWrite(STEP_EN, 0); // No Steps
	analogWriteFrequency(analog_frequency); 

	digitalWrite(STEP_DIR, 0); // Forward.
	bool enabled = false;

	for (;;)
	{
		int tempFrequency = 100; // temp iff no queue to be read.
		if (xQueueReceive(*freqQueue, &tempFrequency, 2) == pdTRUE)
		{
			analog_frequency = tempFrequency;
			Serial.print("New Frequency: ");
			Serial.println(analog_frequency);
		}

		// TOGGLE LOGIC 
		// TODO needs validating for e-stop.
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
