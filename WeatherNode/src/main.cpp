#include <Arduino.h>
#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
//#include <DHTesp.h>


//#define EMULATE_HT
//#define JSON_DATA_FORMAT

#if ESP32
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
#define DHTPIN 12
#define DHTTYPE DHTesp::DHT22//DHT22
#else 
    #ifdef TARGET_ESP01
    #define DHTPIN 0
    #else
    #define DHTPIN D3
    #endif
    #define DHTTYPE DHT22//DHTesp::DHT22
#endif

//DHTesp dht;
DHT _dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

char dsn[32];
char mssg[256];

#define RTC_DATA_CRC 0xDEADBEAF
typedef struct 
{
    uint32_t crc;
    int sleepTime;
    int bootCount;
    int wifi_errors;
} RTCData;

#if ESP32
RTC_DATA_ATTR RTCData rtcData;
#else
RTCData rtcData;
#endif

void reconnect()
{
    while (!client.connected()) {
        if (!client.connect(dsn)) {
            delay(1000);
            Serial.print("*");
        }
        else {
            client.subscribe("config");
        }
    }
}

void loopMqtt(uint32_t time_ms = 2000)
{
    auto sw2 = millis();
    while((millis()-sw2)<=time_ms)
        client.loop();
}

void publishLog(const char* category, const char* message)
{
   // String topic = "/sensor/log/";
    //topic = topic + String(category);
    //sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"cat\":\"%s\",\"mssg\":\"%s\"}", rtcData.bootCount, dsn, category, message);
    //reconnect();
    // if (!client.publish(topic.c_str(), mssg))
    //     Serial.printf("[%s] %s. ####\n\r",topic.c_str(), message);
    // else
        Serial.printf("[%d] (%s) %s\n\r",millis(),category, message);
    // delay(100);
}

void logError(const char* message)
{
    publishLog("ERR", message);
}

void logInfo(const char* message)
{
    publishLog("INF", message);
}

void logLastSensorError()
{
    logError("Failed to read sensor. Time out error!");
}

boolean beginSensor()
{
    #ifndef EMULATE_HT
    //dht.setup(DHTPIN, DHTTYPE);
    _dht.begin();
    #endif
    return true;
}
float readTemp()
{    
    #ifdef EMULATE_HT
    #ifdef ESP32
    return (float)temprature_sens_read();
    #else
    return analogRead(0);
    #endif
    #else
    // TempAndHumidity newValues = dht.getTempAndHumidity();
    // return newValues.temperature;
    return _dht.readTemperature();
    #endif
}
float readHum()
{
    #ifdef EMULATE_HT
    #ifdef ESP32
    return (float)hallRead()/1000;
    #else
    return analogRead(1);
    #endif
    #else
    // TempAndHumidity newValues = dht.getTempAndHumidity();
    // return newValues.humidity;
    return _dht.readHumidity();
    #endif
}

void callback(char* topic, byte* payload, unsigned int length) {
    if (String(topic) == String("config"))
    {
        int newSleepTime = rtcData.sleepTime;
        if (length == 2)
            newSleepTime = payload[0] | (payload[1]<<8);
        else
            newSleepTime = payload[0];
        if (newSleepTime != rtcData.sleepTime)
        {
            rtcData.sleepTime = newSleepTime;
            Serial.printf("Sleep time has been updated to: %d sec\n\r", rtcData.sleepTime);
        }
    }
}

void readRTCData()
{
    #ifndef ESP32
    ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
    #endif
    if (rtcData.crc != RTC_DATA_CRC)
    {
        rtcData.sleepTime = 15;
        rtcData.bootCount = 0;
        rtcData.wifi_errors = 0;
    }
}

void saveRTCData()
{
    rtcData.bootCount++;
    rtcData.crc = RTC_DATA_CRC;
    #ifndef ESP32
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
    #endif
}

void deepSleep(uint32_t time_ms)
{
    time_ms -= 200;
    logInfo((String("Node offline. Deep sleep for ") + String(time_ms) + String(" ms")).c_str());
    saveRTCData();
    delay(200);
    #if ESP32
    //ESP.deepSleep(time_ms*1e3);
    esp_sleep_enable_timer_wakeup(time_ms * 1e3);
    esp_deep_sleep_start();
    #else
    WiFi.disconnect( true );
    delay( 1 );
    #if TARGET_ESP01
    WiFi.forceSleepBegin(time_ms*1e3);
    delay(time_ms);
    WiFi.forceSleepWake();
    ESP.restart();
    #else
    ESP.deepSleep(time_ms*1e3, WAKE_RF_DISABLED );
    #endif
    #endif
}

void restart()
{
    logInfo("Node offline. Reebooting...");
    saveRTCData();
    ESP.restart();
}
void setup_wifi_begin()
{ 
    logInfo("Connecting to Wifi...");
    WiFi.forceSleepWake();
    delay( 1 );
    WiFi.persistent( false );
    WiFi.mode( WIFI_STA );
    IPAddress local_IP(192, 168, 1, 26);
    IPAddress gateway(192, 168, 1, 254);
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.config(local_IP, gateway, subnet)) {
        logError("STA Failed to configure");
    }
    //WiFi.begin("smart-hub-npik2", "smarthome2018");
    WiFi.begin("zza-net", "graphidus2018marcon77deblux");
}
void setup_wifi_wait(unsigned int timeout = 15000)
{ 
    auto sw = millis();
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        if ((millis() - sw) > timeout) {
            logError("Failed to connect to WiFi. Timeout!");
            restart();
        }
    }
}

void setup() {
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    delay( 1 );
    // pinMode(D1, OUTPUT);
    // digitalWrite(D1, HIGH);
    auto sw = millis();
    Serial.begin(115200);
    logInfo("WeatherNode. Firmware v1.0.2");
    #ifdef ESP32
    auto chipid = ESP.getEfuseMac();
    sprintf(dsn,"A%08X%08X",(uint16_t)(chipid>>32), (uint16_t)(chipid));
    #else
    auto chipid = ESP.getChipId();
    sprintf(dsn,"A%08X%08X",0xDE00, chipid);
    #endif
    readRTCData();
    logInfo((String("DSN: ") + String(dsn) + ", boot_count: " + String(rtcData.bootCount)).c_str());
   
    client.setServer("192.168.1.121", 1883);
    client.setCallback(callback);
   
    auto startAt = millis();
    setup_wifi_begin();
    delay(500);
    float t = NAN, h = NAN;
    auto sw1 = millis();
    logInfo("Reading sensor data: ");
    int readErrors = -1;
    while (isnan(h) || isnan(t))
    {
        readErrors++;
        if (beginSensor())
        {
            t = readTemp();
            h = readHum();
            break;
        }
        if ((millis()-sw1) >= 12000)
        {
            logLastSensorError();
            restart();
        }
        delay(1000);
    }
    if (isnan(h)) h =  0.0;
    if (isnan(t)) t =  0.0;
    //digitalWrite(D1, LOW);
    
    setup_wifi_wait();
    auto finishedAt = millis();
    logInfo((String("Connected. IP address: ")+WiFi.localIP().toString()).c_str());
    logInfo((String("WiFi setup time: ")+String(finishedAt-startAt, DEC)).c_str());
    byte mac[6];
    WiFi.macAddress(mac);
    logInfo((String("MAC: ") + String(mac[0],HEX) + ":" + String(mac[1],HEX) + ":" + String(mac[2],HEX) + ":" + String(mac[3],HEX) + ":" + String(mac[4],HEX) + ":" + String(mac[5],HEX)).c_str());

    reconnect();
    #ifdef JSON_DATA_FORMAT
    sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"int\":%d,\"dtp\":\"%s\",\"value\":{\"t\":%.2f,\"h\":%.2f,\"err\":%d,\"wifi_err\":%d}}", rtcData.bootCount, dsn, rtcData.sleepTime, "th", t, h, readErrors, rtcData.wifi_errors);
    #else
    //sprintf(mssg, "%08x%08x%08x%08x",(uint32_t)chipid, (uint32_t)rtcData.bootCount, (uint32_t)(t*1024), (uint32_t)(h*1024));
    sprintf(mssg, "%08x %04d %02.2f %02.2f ",(uint32_t)chipid, (uint32_t)rtcData.bootCount, t, h);
    #endif
    if (client.publish("/sensor/data2", mssg))
        Serial.printf("[%08ld] Message %d sent. JSON: %s\n\r", millis(), rtcData.bootCount, mssg);
    else
        logError("Failed to send message!");
    if (rtcData.crc != RTC_DATA_CRC)
    {
        loopMqtt();
    }
    deepSleep(rtcData.sleepTime * 1e3 - (millis() - sw));
}

void loop() {
}