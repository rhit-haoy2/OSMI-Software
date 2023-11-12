#include <Arduino.h>
// #include "OSMI-StepperDriver/StepperDriver.h"
#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 500

bool debouncing = false;
hw_timer_t *debounce_timer;
QueueHandle_t toggleHandle;

typedef struct StepperMotor
{
  int pins[4];
  int counter;
  bool enabled;
} StepperMotor_t;

typedef struct TestTaskStruct
{
  StepperMotor *motor;
  QueueHandle_t stopHandle;
} TestTaskStruct_t;

void TestTask(void *params)
{
  TestTaskStruct_t *parm = (TestTaskStruct_t *)params;

  for (;;)
  {
    
    bool enable = parm->motor->enabled;
    bool received = xQueueReceive(parm->stopHandle, &enable, 0);
    if (received == pdTRUE)
    {
      parm->motor->enabled = parm->motor->enabled ^ enable;
      Serial.print("Stepper Enabled: ");
      Serial.println(parm->motor->enabled);
    }
    delay(200);
  }
}

/* Interrupt Setup */

void ToggleISR()
{
  if (!debouncing)
  {
    timerRestart(debounce_timer);
    timerStart(debounce_timer);
    debouncing = true;

    BaseType_t toBack = pdTRUE;
    bool toggle = true;

    Serial.print("Sent to Queue ");
    Serial.println(xQueueSendFromISR(toggleHandle, &toggle, &toBack));
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

/* End Debounce */

void setup(void)
{

  Serial.begin(115200);

  initDebounceTimer();

  // Create a dummy motor for the test task.
  StepperMotor_t *motor = (StepperMotor_t *)malloc(sizeof(StepperMotor_t));
  *motor = {.pins = {0, 1, 2, 3}, .counter = 0, .enabled = false};

  toggleHandle = xQueueCreate(1, sizeof(int));

  // Create Test Task Params
  TestTaskStruct_t *testTaskStruct = (TestTaskStruct_t *)malloc(sizeof(TestTaskStruct_t));
  testTaskStruct->motor = motor;
  testTaskStruct->stopHandle = toggleHandle;

  // Create Test Task Struct
  BaseType_t testSuccess = xTaskCreate(TestTask, "Test", configMINIMAL_STACK_SIZE + 512, (void *)testTaskStruct, 1, nullptr);

  // Create Start Stop Button
  pinMode(17, INPUT_PULLUP); // Set pin 17 to pullup input.
  attachInterrupt(digitalPinToInterrupt(17), ToggleISR, RISING);
}

void loop()
{
  delay(5000);
}
