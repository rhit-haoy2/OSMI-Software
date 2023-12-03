#include <esp_wifi.h>
#include <string.h>

#define WIFI_SSID "OSMI-PUMP"

void WIFI_Task(void* params) {

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 2,//consider crying
            

        }
    }

}