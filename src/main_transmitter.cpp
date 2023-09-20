#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

int counter = 0;

void setup() {
    radio.begin();

    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
}

void loop() {
    char text[32];
    sprintf(text, "Counter: %d", counter);
    radio.write(&text, sizeof(text));
    delay(1000);

    counter++;
}
