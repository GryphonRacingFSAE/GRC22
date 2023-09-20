#include <Adafruit_MPU6050.h>
#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

void setup() {
    Serial.begin(115200);
    delay(500);

    while (!Serial)
        delay(10);

    Serial.println("\nInitializing radio...");
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.startListening();
    Serial.println("Done\n");
}

void loop() {
    if (radio.available()) {
        char packet[200] = "";
        radio.read(&packet, sizeof(packet));
        Serial.println(packet);
    }
}
