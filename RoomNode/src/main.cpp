// #include <Arduino.h>
// #ifdef ESP32
// #include <WiFi.h>
// #else
// #include <ESP8266WiFi.h>
// #endif
// #include <PubSubClient.h>
// #include <BMP280.h>

// #ifdef ESP32
// #define SDA_PIN 21
// #define SCL_PIN 22
// #else
// #define SDA_PIN 0
// #define SCL_PIN 2
// #endif

// BMP280 bmp;

// WiFiClient espClient;
// PubSubClient client(espClient);

// char dsn[32];
// char mssg[128];
// int sleepTime = 15;
// int bootCount = 0;

// void reconnect()
// {
//     while (!client.connected()) {
//         if (!client.connect(dsn)) {
//             delay(1000);
//             Serial.print("*");
//         }
//         else {
//             //client.subscribe("config");
//         }
//     }
// }

// void loopMqtt(uint32_t time_ms = 2000)
// {
//     auto sw2 = millis();
//     while((millis()-sw2)<=time_ms)
//         client.loop();
// }

// void publishLog(const char* category, const char* message)
// {
//     String topic = "/sensor/log/";
//     topic = topic + String(category);
//     sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"cat\":\"%s\",\"mssg\":\"%s\"}", bootCount, dsn, category, message);
//     reconnect();
//     if (!client.publish(topic.c_str(), mssg))
//         Serial.printf("[%s] %s. ####\n\r",topic.c_str(), message);
//     else
//         Serial.printf("[%s] %s\n\r",category, message);
// }

// void logError(const char* message)
// {
//     publishLog("ERR", message);
// }

// void logInfo(const char* message)
// {
//     publishLog("INF", message);
// }

// void callback(char* topic, byte* payload, unsigned int length) {
//     if (String(topic) == String("config"))
//     {
//         int newSleepTime = sleepTime;
//         if (length == 2)
//             newSleepTime = payload[0] | (payload[1]<<8);
//         else
//             newSleepTime = payload[0];
//         if (newSleepTime != sleepTime)
//         {
//             sleepTime = newSleepTime;
//             Serial.printf("Sleep time has been updated to: %d sec\n\r", sleepTime);
//         }
//     }
// }

// void deepSleep(int time_ms)
// {
//     if (time_ms <= 0) time_ms = sleepTime*1e3;
//     bootCount++;
//     logInfo("Node offline. Deep sleep...");
//     delay(100);
//     #ifdef ESP32
//     ESP.deepSleep(time_ms*1e3);
//     #else
//     WiFi.forceSleepBegin(time_ms*1e3);
//     delay(time_ms);
//     WiFi.forceSleepWake();
//     #endif
// }

// void restart()
// {
//     logInfo("Node offline. Rebooting...");
//     loopMqtt(1000);
//     ESP.restart();
// }

// void setup_wifi(unsigned int timeout = 5000)
// { 
//     Serial.println("Connecting to Wifi...");
//     WiFi.persistent(false);
//     WiFi.mode(WIFI_OFF);
//     WiFi.mode(WIFI_STA);
//     //WiFi.begin("smart-hub-npik2", "smarthome2018");
//     WiFi.begin("zza-net", "graphidus2018marcon77deblux");
//     auto sw = millis();
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(1000);
//         Serial.print(".");
//         if ((millis() - sw) > timeout) {
//             logError("Failed to connect to WiFi. Timeout!");
//             restart();
//         }
//     }
//     Serial.println();
//     Serial.print("Connected, IP address: ");
//     Serial.println(WiFi.localIP());
//     String online = String("Node online: ") + WiFi.localIP().toString();
//     logInfo(online.c_str());
// }

// void setup() {
//     Serial.begin(115200);
//     #ifdef ESP32
//     auto chipid = ESP.getEfuseMac();
//     sprintf(dsn,"A%08X%08X",(uint16_t)(chipid>>32), (uint16_t)(chipid));
//     #else
//     auto chipid = ESP.getChipId();
//     sprintf(dsn,"A%08X%08X",0xDE00, chipid);
//     #endif
//     Serial.printf("\n\rdsn:%s, firmware v1.0.0b\n\r", dsn);
//     client.setServer("192.168.1.121", 1883);
//     client.setCallback(callback);

//     setup_wifi();    

//      if (bmp.begin(SDA_PIN, SCL_PIN)){
//         bmp.setOversampling(4);
//      }
//      else {
//         logError("Failed to init sensor!");
//         restart();
//      }
// }

// void loop() {
//     int readErrors = 0;
//     double t = NAN,p = NAN;
//     auto sw = millis();
//     setup_wifi();    
//     auto sw1 = millis();
//     int errCode = 0;
//     while (isnan(p) || isnan(t))
//     {
//         char result = bmp.startMeasurment();
//         if(result != 0){
//             delay(result);
//             result = bmp.getTemperatureAndPressure(t,p);
//             if(result == 1){
//                 break;
//             }
//             else
//                 errCode = -2;
//         }
//         else
//             errCode = -1;
        
//         if ((millis()-sw1) >= 12000)
//         {
//             if (errCode == -2)
//                 logError("Failed to read sensor data!");
//             else if (errCode == -1)
//                 logError("Failed to start measurment!");
//             restart();
//         }
//         readErrors++;
//         delay(1000);
//     }
//     if (isnan(t)) t =  0.0;
//     if (isnan(p)) p =  0.0;

//     reconnect();
//     sprintf(mssg, "{\"seq\":%d,\"dsn\":\"%s\",\"int\":%d,\"dtp\":\"%s\",\"value\":{\"t\":%.2f,\"p\":%.2f,\"err\":%d}}", bootCount, dsn, sleepTime, "tp", t, p, readErrors);
//     if (client.publish("/sensor/data", mssg))
//         Serial.printf("[%08ld] Message %d sent. JSON: %s\n\r", millis(), bootCount, mssg);
//     else
//         logError("Failed to send message!");
//     loopMqtt();

//     deepSleep(sleepTime * 1e3 - (millis() - sw));
// }

#include <Arduino.h>
#include <Wire.h>
#include <BMP280.h>
BMP280 bmp;

void setup()
{ 
    Serial.begin(115200);
    if(!bmp.begin(21,22)){
        Serial.println("BMP init failed!");
        while(1);
    }
    else Serial.println("BMP init success!");
    
    bmp.setOversampling(4);
}

void loop()
{
    double T,P;
    char result = bmp.startMeasurment();
    
    if(result!=0){
        delay(result);
        result = bmp.getTemperatureAndPressure(T,P);
        
        if(result!=0)
        {
            Serial.print("T = \t");Serial.print(T,2); Serial.print(" degC\t");
            Serial.print("P = \t");Serial.print(P,2); Serial.print(" mBar\t");
            Serial.println();
        }
        else {
            Serial.println("Error.");
        }
    }
    else {
        Serial.println("Error.");
    }
    delay(1000);
}