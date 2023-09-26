#include <Adafruit_MPU6050.h>
#include <Arduino.h>
#include <HardwareSerial.h>
#include <RF24.h>
#include <TinyGPSPlus.h>
#include <pb_encode.h>

#include "MPU6050.pb.h"

RF24 radio(4, 5); // CE, CSN
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;

HardwareSerial GPSSerial(2);

const byte address[6] = "00001";

void setup() {
    Serial.begin(115200);
    delay(500);

    while (!Serial)
        delay(10);

    Serial.println("\nInitializing NRF24L01...");
    radio.begin();
    radio.openWritingPipe(address);
    radio.stopListening();
    Serial.println("Done");

    Serial.println("Initializing MPU6050...");
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2, 4, 8, 16
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);      // 250, 500, 1000, 2000
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);   // 260, 184, 94, 44, 21, 10, 5
    Serial.println("Done");

    Serial.println("Initializing BN-220...");
    GPSSerial.begin(4800);
    Serial.println("Done");
}

void loop() {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);

    MPU6050_Message mpu_msg = MPU6050_Message_init_default;

    // Values are opposite for some reason (but it works)
    mpu_msg.acceleration_x = g.gyro.x;
    mpu_msg.acceleration_y = g.gyro.y;
    mpu_msg.acceleration_z = g.gyro.z;
    mpu_msg.rotation_x = a.acceleration.x;
    mpu_msg.rotation_y = a.acceleration.y;
    mpu_msg.rotation_z = a.acceleration.z;

    uint8_t buffer[128];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&stream, MPU6050_Message_fields, &mpu_msg);

    radio.write(buffer, stream.bytes_written);

    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        Serial.print(c);
        if (gps.encode(c)) {
            Serial.println("C");
            if (gps.location.isValid()) {
                Serial.println("D");
                float latitude = gps.location.lat();
                float longitude = gps.location.lng();

                Serial.print("Latitude: ");
                Serial.println(latitude, 6);
                Serial.print("Longitude: ");
                Serial.println(longitude, 6);
            }
        }
    }

    delay(100);
}
