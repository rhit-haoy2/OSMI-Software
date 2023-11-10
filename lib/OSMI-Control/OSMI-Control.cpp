#include "OSMI-Control.h"

void SetupControl(void *params)
{
  float setpoint = 0; 
  float posit = 0;
  
  //resistance init
  int raw = 0;
  int Vin = 5;
  float Vout = 0;
  float R1 = 10000; //known resistance
  float R = 0; //unknown resistance
  float buffer = 0;

  //control system init
  float K = 100; //tuning parameter
  float error = 0;
  float gain = 0;
    Serial.begin(9600);
}

void LoopControl()
{
      //position
  posit = analogRead(A0); //reads value of the potentiometer
  Serial.print("position: ");
  Serial.println(posit);
  setpoint = ((posit-525)/498)*10000;
  if (setpoint < 0) {
    setpoint = 0;
  }
  Serial.print("setpoint: ");
  Serial.println(setpoint);
   
  //resistance
  raw = analogRead(A0);
  if(raw){
    buffer = raw * Vin;
    Vout = (buffer)/1024.0;
    buffer = (Vin/Vout) - 1;
    R = R1 * buffer;
    Serial.print("PV: ");
    Serial.println(R);
  }
  error = setpoint - R;
  Serial.print("error: ");
  Serial.println(error);
  gain = K*error;
  Serial.print("gain: ");
  Serial.println(gain);
  delay(3000);
}
