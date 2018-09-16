#include <Arduino.h>
#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#if ESP32
#define DHTPIN 15
#define DHTTYPE DHT11
#else 
    #ifdef TARGET_ESP01
    #define DHTPIN 0
    #else
    #define DHTPIN D4
    #endif
    #define DHTTYPE DHT22
#endif

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
    String topic = "/sensor/log/";
    topic = topic + String(category);
    sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"cat\":\"%s\",\"mssg\":\"%s\"}", rtcData.bootCount, dsn, category, message);
    reconnect();
    if (!client.publish(topic.c_str(), mssg))
        Serial.printf("[%s] %s. ####\n\r",topic.c_str(), message);
    else
        Serial.printf("[%s] %s\n\r",category, message);
    delay(100);
}

void logError(const char* message)
{
    //publishLog("ERR", message);
}

void logInfo(const char* message)
{
    //publishLog("INF", message);
}

void logLastSensorError()
{
    logError("Failed to read sensor. Time out error!");
}

boolean beginSensor()
{
    Serial.print(".");
    _dht.begin();
    return true;
}
float readTemp()
{
    return _dht.readTemperature();
}
float readHum()
{
    return _dht.readHumidity();
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
    ESP.deepSleep(time_ms*1e3);
    #else
    WiFi.forceSleepBegin();
    WiFi.mode(WIFI_OFF);
    #if TARGET_ESP01
    WiFi.forceSleepBegin(time_ms*1e3);
    delay(time_ms);
    WiFi.forceSleepWake();
    ESP.restart();
    #else
    ESP.deepSleep(time_ms*1e3, WAKE_RF_DEFAULT);
    #endif
    #endif
}

void restart()
{
    logInfo("Node offline. Reebooting...");
    saveRTCData();
    ESP.restart();
}

void setup_wifi(unsigned int timeout = 15000)
{ 
    Serial.println("Connecting to Wifi...");
    if (!WiFi.isConnected())
    {
        WiFi.forceSleepWake();
        WiFi.mode(WIFI_STA);
        //WiFi.begin("smart-hub-npik2", "smarthome2018");
        WiFi.begin("zza-net", "graphidus2018marcon77deblux");
        auto sw = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);
            Serial.print(".");
            if ((millis() - sw) > timeout) {
                logError("Failed to connect to WiFi. Timeout!");
                restart();
            }
        }
        Serial.println();
        Serial.print("Connected, IP address: ");
    }
    else {
        Serial.print("Already connected, IP address: ");
    }
    logInfo((String("Node online. IP: ")+WiFi.localIP().toString()).c_str());
}

void setup() {

    auto sw = millis();
    Serial.end();
    //Serial.begin(115200);
    #ifdef ESP32
    auto chipid = ESP.getEfuseMac();
    sprintf(dsn,"A%08X%08X",(uint16_t)(chipid>>32), (uint16_t)(chipid));
    #else
    auto chipid = ESP.getChipId();
    sprintf(dsn,"A%08X%08X",0xDE00, chipid);
    #endif
    readRTCData();
    Serial.printf("\n\rdsn:%s, firmware v1.0.1a [boot_count=%d]\n\r", dsn, rtcData.bootCount);
   
    client.setServer("192.168.1.121", 1883);
    client.setCallback(callback);
   
    float t = NAN, h = NAN;
    auto sw1 = millis();
    Serial.print("Reading sensor data: ");
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
            Serial.println();
            logLastSensorError();
            restart();
        }
        delay(1000);
    }
    Serial.println();
    if (isnan(h)) h =  0.0;
    if (isnan(t)) t =  0.0;

    
    setup_wifi();
    reconnect();
    //sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"int\":%d,\"dtp\":\"%s\",\"value\":{\"t\":%.2f,\"h\":%.2f,\"err\":%d,\"wifi_err\":%d}}", rtcData.bootCount, dsn, rtcData.sleepTime, "th", t, h, readErrors, rtcData.wifi_errors);
    sprintf(mssg, "%08x%08x%08x%08x",(uint32_t)chipid, (uint32_t)rtcData.bootCount, (uint32_t)(t*1024), (uint32_t)(h*1024));
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