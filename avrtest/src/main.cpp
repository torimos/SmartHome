#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#define CS_PIN 5
#define CE_PIN 17
#define RFTX1

RF24 radio(CE_PIN, CS_PIN);

uint8_t address[11] = "SimpleNode";
unsigned long payload = 0;
int i = 0;

#ifdef RFTX

void setup() {
    Serial.begin(115200);
    radio.begin();
    SPI.begin(18, 19, 23, 5);
    radio.setAutoAck(1);
    radio.setRetries(15,15);
    radio.openWritingPipe(address);

    if (!radio.isChipConnected())
    {
        Serial.println("The nRF24L01 is not connected!");
        while(1);
    }
}

void loop() {
    payload++;
    radio.write( &payload, sizeof(unsigned long) ); //Send data to 'Receiver' ever second
    Serial.printf("Sent payload: %d\n\r", payload);
    delay(1000);
}

#else

void setup() {
    Serial.begin(115200);
    radio.begin();
    SPI.begin(18, 19, 23, 5);
    radio.setAutoAck(1);
    radio.setRetries(15,15);
    radio.openReadingPipe(1, address);
    radio.startListening();
    if (!radio.isChipConnected())
    {
        Serial.println("The nRF24L01 is not connected!");
        while(1);
    }
}
void loop(void){
    radio.stopListening();
    radio.startListening();
    //if (radio.available())
    {
        radio.read( &payload, sizeof(unsigned long) );
        if(payload != 0){
            Serial.printf("%06d Got Payload ", i++);
            Serial.println(payload);
        }
        delay(1000);
    }
}

#endif