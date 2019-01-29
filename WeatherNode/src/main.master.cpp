#include "main.h"

#if IS_MASTER

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "config.h"
#include <EEPROM.h>
#define CONFIG_PIN 14

ADC_MODE(ADC_VCC);

Adafruit_BME280 bme;

TSensorData sd;
MasterConfig _cfg;

void scan()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(50);
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
     
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.print(" ch=");
      Serial.print(WiFi.channel(i));
      Serial.print((WiFi.encryptionType(i) == 0) ? " " : "*");
      Serial.print(" MAC=");
      Serial.println(WiFi.BSSIDstr(i));

      if (WiFi.SSID(i) == HOST_NAME)
      {
        memcpy(_cfg.host_mac, WiFi.BSSID(i), 6);
        break;
      }

      delay(10);
    }
  }
  Serial.println();
}

void sendData(uint8_t mac[6], uint8_t* data, size_t dataLen)
{
  Serial.printf("[%ld] Sending ESP-NOW message to %02X:%02X:%02X:%02X:%02X:%02X\n\r", millis(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  check(WifiEspNow.send(mac, data, dataLen), "WifiEspNow.send(...) failed");
}

void deepSleep(uint32_t time_ms)
{
    time_ms -= 200;
    Serial.println((String("Node offline. Deep sleep for ") + String(time_ms) + String(" ms")).c_str());
    delay(200);
    #if ESP32
    esp_sleep_enable_timer_wakeup(time_ms * 1e3);
    esp_deep_sleep_start();
    #else
    WiFi.disconnect( true );
    delay( 1 );
    WiFi.forceSleepBegin(time_ms*1e3);
    ESP.deepSleep(time_ms*1e3, WAKE_RF_DEFAULT );
    #endif
}

void readSensorData()
{
  sd.timestamp = millis();
  sd.v = ESP.getVcc();
  if (bme.begin(0x76))
  {
    sd.t = bme.readTemperature();
    sd.h = bme.readHumidity();
    sd.p = bme.readPressure();
  }
  else
  {
    sd.t = 0xDE;
    sd.h = 0xAD;
    sd.p = 0xBE;
    Serial.println("Could not find a valid BMP280 sensor, check wiring! Proceed with fake vaules");
  }
}

long sw_start = 0;
void setup()
{
  sw_start = millis();

  pinMode(CONFIG_PIN, INPUT_PULLUP);
  WiFi.mode( WIFI_OFF );
    
  #if defined(ESP8266)
  WiFi.forceSleepBegin();
  delay( 1 );
  #endif

  Serial.begin(115200);

  readSensorData();

  #if defined(ESP8266)
  check(WiFi.forceSleepWake(), "WiFi.forceSleepWake() failed"); 
  delay( 1 );
  WiFi.persistent( false );
  #endif
  check(WiFi.mode(WIFI_STA), "WiFi.mode(WIFI_STA) failed");
  
  if (!_cfg.load() || !digitalRead(CONFIG_PIN))
  {
    scan();
    _cfg.save();
  }

  espNowInitMaster(_cfg.host_mac, nullptr);
  
  for (int z=0;z<1;z++) {
    sendData(_cfg.host_mac, (uint8_t*) &sd, sizeof(sd));
    delay( 1 );
  }
  auto sw_end = millis() - sw_start;
  Serial.printf("Time = %ld t=%.3f h=%.3f p=%.3f v=%d\n\r", sw_end, sd.t, sd.h, sd.p, sd.v);
  deepSleep(1800000);
}

void loop() {
}

#endif