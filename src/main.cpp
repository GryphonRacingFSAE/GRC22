#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

#ifdef TRANSMITTER_CODE
int counter = 0;
#endif

void setup() {
    radio.begin();

#ifdef TRANSMITTER_CODE
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
#endif

#ifdef RECEIVER_CODE
    Serial.begin(115200);

    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
#endif
}

void loop() {
#ifdef TRANSMITTER_CODE
    char text[32];
    sprintf(text, "Counter: %d", counter);
    radio.write(&text, sizeof(text));
    delay(1000);

    counter++;
#endif

#ifdef RECEIVER_CODE
    if (radio.available()) {
        char text[32] = "";
        radio.read(&text, sizeof(text));
        Serial.println(text);
    }
#endif
}
