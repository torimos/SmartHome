#include "main.h"

#if !IS_MASTER

void processRx(const uint8_t mac[6], const uint8_t* buf, size_t count, void* cbarg) {
  // Serial.printf("[%d] Message from %02X:%02X:%02X:%02X:%02X:%02X = ", millis(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // for (uint i = 0; i < count; ++i) {
  //   Serial.printf("%02X ", buf[i]);
  // }
  // Serial.println();
  TSensorData *p = (TSensorData *)buf;
  Serial.printf("#{\"ts\":%ld,\"t\":%.3f,\"h\":%.3f,\"p\":%.3f,\"v\":%d,\"dsn\":\"%02X%02X%02X%02X%02X%02X\"}\n", millis(), p->t, p->h, p->p, p->v, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void setup()
{
  Serial.begin(115200);
  check(WiFi.mode(WIFI_AP), "WiFi.mode(WIFI_AP) failed"); 
  check(WiFi.softAP(HOST_NAME, nullptr, CHANNEL), "WiFi.mode(WIFI_AP) failed");
  
  espNowInitSlave(processRx);
  
  getMac();
}

void loop() {
}

#endif