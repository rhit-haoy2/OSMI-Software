#include <Arduino.h>
// #include "OSMI-StepperDriver/StepperDriver.h"
#define DEBOUNCE_TIMER_ID 1
#define DEBOUNCE_PRESCALE 20000
#define DEBOUNCE_THRESHOLD 500

bool debouncing = false;
hw_timer_t* debounce_timer;

typedef struct StepperMotor {
  int pins[4];
  int counter;
} StepperMotor_t;


void TestTask(void *params)
{
  StepperMotor_t ass = *(StepperMotor_t *)params;

  for (;;)
  {
  Serial.println("Pain");
  delay(1000);
  }
}

void ToggleISR() {
  if(!debouncing) {
    Serial.println("Button");
    timerRestart(debounce_timer);
    timerStart(debounce_timer);
    debouncing = true;
  } else {
    Serial.println("Still Debouncing");
    Serial.println(timerRead(debounce_timer));
  }
  
}

void IRAM_ATTR DebounceToggleISR() {
  timerStop(debounce_timer);
  Serial.println("Debounced");
  debouncing = false;
}

void setup(void)
{

  Serial.begin(115200);

  debounce_timer = timerBegin(DEBOUNCE_TIMER_ID, DEBOUNCE_PRESCALE, true);
  timerStop(debounce_timer);
  timerAttachInterrupt(debounce_timer, &DebounceToggleISR, true);
  timerAlarmWrite(debounce_timer, DEBOUNCE_THRESHOLD, true);
  timerAlarmEnable(debounce_timer);

  StepperMotor_t* motor = (StepperMotor_t*) malloc(sizeof(StepperMotor_t));
  *motor = {.pins={0,1,2,3}, .counter=0};

  BaseType_t testSuccess = xTaskCreate(TestTask, "Test", configMINIMAL_STACK_SIZE, (void *)motor, 1, nullptr);
  // int testSuccess = 1;

  pinMode(17, INPUT_PULLUP); // Set pin 17 to pullup input.

  attachInterrupt(digitalPinToInterrupt(17), ToggleISR, RISING);

  Serial.println(testSuccess == 0);
}

void loop()
{
  delay(5000);
}

