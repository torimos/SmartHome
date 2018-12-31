#include "main.h"

void getMac()
{
    Serial.print("SoftAP MAC address of this node is ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("MAC address of this node is ");
    Serial.println(WiFi.macAddress());
}

void check(bool result, const char *error_mssg)
{
    if (!result) {
        Serial.println(error_mssg);
        delay(30000);
        ESP.restart();
    }
}

void espNowInitMaster(const uint8_t *mac, WifiEspNowClass::RxCallback cb)
{
    #if defined(ESP8266)
    check(WifiEspNow.begin(esp_now_role::ESP_NOW_ROLE_CONTROLLER), "WifiEspNow.begin() failed");
    check(WifiEspNow.addPeer(mac, CHANNEL, esp_now_role::ESP_NOW_ROLE_SLAVE), "WifiEspNow.addPeer(...) failed");
    #elif defined(ESP32)
    check(WifiEspNow.begin(), "WifiEspNow.begin() failed");
    check(WifiEspNow.addPeer(mac, CHANNEL), "WifiEspNow.addPeer(...) failed");
    #endif

    if (cb)
        WifiEspNow.onReceive(cb, nullptr);
}

void espNowInitSlave(WifiEspNowClass::RxCallback cb)
{
    #if defined(ESP8266)
    check(WifiEspNow.begin(esp_now_role::ESP_NOW_ROLE_COMBO), "WifiEspNow.begin() failed");
    #elif defined(ESP32)
    check(WifiEspNow.begin(), "WifiEspNow.begin() failed");
    #endif

    if (cb)
        WifiEspNow.onReceive(cb, nullptr);
}