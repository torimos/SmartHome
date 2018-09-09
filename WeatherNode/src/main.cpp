#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 15
#define DHTTYPE DHT22

DHT _dht(DHTPIN, DHTTYPE);
WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

char dsn[32];
char mssg[128];
RTC_DATA_ATTR int sleepTime = 15;
RTC_DATA_ATTR int bootCount = 0;

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
    sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"cat\":\"%s\",\"mssg\":\"%s\"}", bootCount, dsn, category, message);
    reconnect();
    if (!client.publish(topic.c_str(), mssg))
        Serial.printf("[%s] %s. ####\n\r",topic.c_str(), message);
    else
        Serial.printf("[%s] %s\n\r",category, message);
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
        int newSleepTime = sleepTime;
        if (length == 2)
            newSleepTime = payload[0] | (payload[1]<<8);
        else
            newSleepTime = payload[0];
        if (newSleepTime != sleepTime)
        {
            sleepTime = newSleepTime;
            Serial.printf("Sleep time has been updated to: %d sec\n\r", sleepTime);
        }
    }
}

void deepSleep(uint32_t time_ms)
{
    bootCount++;
    Serial.printf("Going to sleep for %d usec...\n\r", time_ms);
    delay(100);
    ESP.deepSleep(time_ms*1e3);
}

void restart()
{
    Serial.println("Rebooting...\n\r");
    loopMqtt(5000);
    ESP.restart();
}

void setup_wifi(unsigned int timeout = 5000)
{ 
    Serial.println("Connecting to Wifi...");
    wifiMulti.addAP("smart-hub-npik2", "smarthome2018");
    wifiMulti.addAP("zza-net", "graphidus2018marcon77deblux");

    if(wifiMulti.run(timeout) == WL_CONNECTED) 
    {
        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        logError("Failed to connect to WiFi. Timeout!");
        restart();
    }
}

void setup() {
    auto sw = millis();
    Serial.begin(115200);
    auto chipid = ESP.getEfuseMac();
    client.setServer("192.168.1.121", 1883);
    client.setCallback(callback);
    
    sprintf(dsn,"A%08X%08X",(uint16_t)(chipid>>32), (uint16_t)(chipid));
    Serial.printf("\n\rdsn:%s, firmware v1.0.0a\n\r", dsn);
    setup_wifi();
    float t = NAN, h = NAN;
    auto sw1 = millis();
    Serial.print("Reading sensor data: ");
    int readErrors = 0;
    while (isnan(h) || isnan(t))
    {
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
        readErrors++;
        delay(1000);
    }
    Serial.println();
    if (isnan(h)) h =  0.0;
    if (isnan(t)) t =  0.0;

    reconnect();
    sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"int\":%d,\"dtp\":\"%s\",\"value\":{\"t\":%.2f,\"h\":%.2f,\"err\":%d}}", bootCount, dsn, sleepTime, "temphum", t, h, readErrors);
    if (client.publish("/sensor/data", mssg))
        Serial.printf("[%08ld] Message %d sent. JSON: %s\n\r", millis(), bootCount, mssg);
    else
        logError("Failed to send message!");
    loopMqtt();

    deepSleep(sleepTime * 1e3 - (millis() - sw));
}

void loop() {
}