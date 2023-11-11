#include <Arduino.h>
#include <string.h>

#define CLOCKWISE 0
#define COUNTERCLOCKWISE 1

class StepperMotor { 
    // Access specifier 
public: 
    //define the 4 ports
    int port1;
    int port2;
    int port3;
    int port4;
    // this boolean says if this motor is on
    bool playornot;
    // state is the state of the four ports
    int state;
    // time for one!! step
    int steptime;
    // clockwise or counterclockwise
    int direction;
    // time for the next step
    int nextsteptime;
    // not used right now
    int stoptime;
    // create a stepper
    StepperMotor(int port1, int port2, int port3, int port4, int speed, int direction){
        this->port1 = port1;
        this->port2 = port2;
        this->port3 = port3;
        this->port4 = port4;
        pinMode(this->port1, OUTPUT);
        pinMode(this->port2, OUTPUT);
        pinMode(this->port3, OUTPUT);
        pinMode(this->port4, OUTPUT);
        delay(30);
        this->playornot = false;
        this->state = 0;
        this->steptime = speed;
        this->direction = direction;
    }
    // toggling the playornot
    void StopStepper(){
        this->playornot = false;
    }
    void StartStepper(){
        this->playornot = true;
    }

    // perform step once!!
    void Step(){
        if(this->playornot == false){
            return;
        }
        this->nextsteptime = this->nextsteptime + this->steptime;
        if(this->state<0){
            this->state = 7;
        }else if(this->state>7){
            this->state = 0;
        }
        switch(state){
            case 0: 
                digitalWrite(this->port1,1);
                digitalWrite(this->port2,0);
                digitalWrite(this->port3,0);
                digitalWrite(this->port4,1);
            break;
            case 1:
                digitalWrite(this->port1,0);
                digitalWrite(this->port2,0);
                digitalWrite(this->port3,0);
                digitalWrite(this->port4,1);
      
            break;
            case 2: 
                digitalWrite(this->port1,0);
                digitalWrite(this->port2,0);
                digitalWrite(this->port3,1);
                digitalWrite(this->port4,1);
            break;
            case 3: 
                digitalWrite(this->port1,0);
                digitalWrite(this->port2,0);
                digitalWrite(this->port3,1);
                digitalWrite(this->port4,0);
            break;
            case 4: 
                digitalWrite(this->port1,0);
                digitalWrite(this->port2,1);
                digitalWrite(this->port3,1);
                digitalWrite(this->port4,0);
            break;
            case 5: 
                digitalWrite(this->port1,0);
                digitalWrite(this->port2,1);
                digitalWrite(this->port3,0);
                digitalWrite(this->port4,0);
            break;
            case 6: 
                digitalWrite(this->port1,1);
                digitalWrite(this->port2,1);
                digitalWrite(this->port3,0);
                digitalWrite(this->port4,0);
            break;
            case 7: 
                digitalWrite(this->port1,1);
                digitalWrite(this->port2,0);
                digitalWrite(this->port3,0);
                digitalWrite(this->port4,0);
            break;
            default: state = 0;
        }
        if(this->direction == CLOCKWISE){
            state++;
        }else if(this->direction == COUNTERCLOCKWISE){
            state--;
        }
        
        //Serial.println(state);
    }
}; 


// make one motor start, toggle its playornot, set first steptime
void StartMotor(StepperMotor motor){
    motor.StartStepper();
    motor.nextsteptime = millis()+motor.steptime;
    
}

// running stage of the motor, keeps updating the nextsteptime
void RunMotor(StepperMotor motor){
    while(motor.playornot == true){
        
        int currenttime = millis();
        Serial.println(currenttime);
        while(currenttime - motor.nextsteptime> motor.steptime){
            motor.Step();
            motor.nextsteptime = motor.nextsteptime + motor.steptime;
        }
        delay(15);
    }
}






