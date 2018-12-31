#include <Arduino.h>
#include <WifiEspNow.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#elif defined(ESP32)
#include <WiFi.h>
#endif

#define IS_MASTER 1
#define CHANNEL 1

#define HOST_NAME "SHCOM-01"

struct __attribute__((packed)) TSensorData
{
  float t;
  float h;
  float p;
  unsigned long timestamp;
};

void getMac();
void check(bool result, const char *error_mssg);
void espNowInitMaster(const uint8_t *mac, WifiEspNowClass::RxCallback cb);
void espNowInitSlave(WifiEspNowClass::RxCallback cb);