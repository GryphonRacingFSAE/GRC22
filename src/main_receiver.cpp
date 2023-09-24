#include <Arduino.h>
#include <RF24.h>
#include <pb_decode.h>

#include "message.pb.h"

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
        uint8_t buffer[128];
        radio.read(&buffer, sizeof(buffer));

        pb_istream_t istream;
        istream = pb_istream_from_buffer(buffer, sizeof(buffer));

        TestMessage msg = TestMessage_init_zero;
        pb_decode(&istream, &TestMessage_msg, &msg);

        Serial.println(msg.number);
    }
}
