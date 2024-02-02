
#include <Arduino.h>
#include "SelfStart.h"
#include "OSMI-Display/OSMI-Display.h"
#include "OSMI-Control/OSMI-Control.h"
#include "OSMI-WIFI/OSMI-WIFI.h"
// #include "OSMI-StepperDriver/StepperDriver.h"
#include "driver/ledc.h"
#include "OSMI-Control/FluidDeliveryController.h"


void SelfStartTestTask(void *params){
    //Todo: getnvs status
    esp_err_t err = nvs_flash_init();
    nvs_handle_t my_handle;
    int8_t status;
    status = nvs_get_i8(my_handle,"Status",&status);

    status = EspDriverStatus_t::limitStopped;


    switch (status)
    {
    case EspDriverStatus_t::Bolus:
    case EspDriverStatus_t::Infusion:
        //Todo: display this on the screen instead of the Serial Monitor
        Serial.println("Last time still moving. Warning!!");
        break;
    default:
        //Todo: modify main.cpp, pass in the display config
        display_config_t *displayConfig = (display_config_t *) params;
        BaseType_t dispSuccess = xTaskCreate(DisplayTask, "DISP", 64000, &displayConfig, 2, nullptr);
        vTaskDelete(NULL); // Delete POST Task.
        break;
    }


}

