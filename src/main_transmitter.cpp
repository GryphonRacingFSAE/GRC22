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

HardwareSerial SerialGPS(1);

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
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
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

    while (SerialGPS.available() > 0) {
        if (gps.encode(SerialGPS.read())) {
            Serial.print("LAT=");
            Serial.println(gps.location.lat(), 6);
            Serial.print("LNG=");
            Serial.println(gps.location.lng(), 6);
            Serial.print("ALT=");
            Serial.println(gps.altitude.meters());
            Serial.println();
        }
    }

    delay(100);
}
