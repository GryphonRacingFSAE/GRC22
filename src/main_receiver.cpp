#include <Arduino.h>
#include <RF24.h>
#include <pb_decode.h>

#include "BN220.pb.h"
#include "MPU6050.pb.h"

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

        pb_istream_t stream = pb_istream_from_buffer(buffer, sizeof(buffer));

        if (BN220_Message_fields != NULL) {
            Serial.println("A");
            BN220_Message msg = BN220_Message_init_default;
            pb_decode(&stream, BN220_Message_fields, &msg);

        } else if (MPU6050_Message_fields != NULL) {
            Serial.println("B");
            MPU6050_Message msg = MPU6050_Message_init_default;
            pb_decode(&stream, MPU6050_Message_fields, &msg);

            Serial.printf("Acceleration:  X %.2f\tY %.2f\tZ %.2f\n", msg.acceleration_x, msg.acceleration_y, msg.acceleration_z);
            Serial.printf("Rotation:      X %.2f\tY %.2f\tZ %.2f\n\n", msg.rotation_x, msg.rotation_y, msg.rotation_z);
        } else {
            Serial.println("Unknown message detected!");
        }
    }
}
