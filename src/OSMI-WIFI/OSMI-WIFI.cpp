#include "OSMI-WIFI.h"
#include <esp_wifi.h>
#include <string.h>
#include <Arduino.h>
#include <WiFi.h>


wl_status_t status = WL_IDLE_STATUS;
WiFiServer server(80);

/// @brief
/// @param params
void WIFI_Task(void *params)
{

    Serial.print("Attempting to connect to: ");
    char ssid[] = WIFI_SSID;
    Serial.println(ssid);
    char pswd[] = WIFI_PSWD;


    status = WiFi.begin(ssid, pswd);    
    while (1) // Connection Loop
    {
        if (status != WL_CONNECTED)
        {
            status = WiFi.status();
            Serial.println("Failed to connect to WiFi Network");
            Serial.print("Status: ");
            Serial.println(status);
            WiFi.disconnect();
            WiFi.reconnect();
            delay(5000);
        } else
        {
            break;
        }
    }

    server.begin();
    Serial.print("Address: ");
    IPAddress addr = WiFi.localIP();

    while(1) {
        Serial.println(addr);
        WiFiClient client = server.available();
        if (client) {
            client.println("You have reached the ESP32"); //Client dropped after this.
        }

        delay(1000);
    }
    
}