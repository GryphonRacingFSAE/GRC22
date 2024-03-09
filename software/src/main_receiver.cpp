#include <Arduino.h>
#include <RF24.h>
#include <pb_decode.h>

#include "CAN.pb.h"

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

void setup() {
    Serial.begin(115200);
    delay(500);

    while (!Serial)
        delay(100);

    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }
    radio.openReadingPipe(0, address);
    radio.startListening();
}

void loop() {
    /*
    if (radio.available()) {
        uint8_t buffer[128];
        radio.read(&buffer, sizeof(buffer));

        MyMessage msg = MyMessage_init_default;

        pb_istream_t stream = pb_istream_from_buffer(buffer, sizeof(buffer));
        pb_decode(&stream, MyMessage_fields, &msg);

        Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.2f\r\n",
                      msg.acceleration_x,
                      msg.acceleration_y,
                      msg.acceleration_z,
                      msg.rotation_x,
                      msg.rotation_y,
                      msg.rotation_z,
                      msg.latitude,
                      msg.longitude,
                      msg.altitude);
    }
    */
}
