#include <Adafruit_MPU6050.h>
#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 radio(4, 5); // CE, CSN
Adafruit_MPU6050 mpu;

const byte address[6] = "00001";

void setup() {
    Serial.begin(115200);
    delay(500);

    while (!Serial)
        delay(10);

    Serial.println("\nInitializing radio...");
    radio.begin();
    radio.openWritingPipe(address);
    radio.stopListening();
    Serial.println("Done");

    Serial.println("Initializing MPU6050...");
    if (!mpu.begin()) {
        Serial.println("Failed to initialize MPU6050!");
        while (1) {
        }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2, 4, 8, 16
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);      // 250, 500, 1000, 2000
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);    // 260, 184, 94, 44, 21, 10, 5
    Serial.println("Done\n");
}

void loop() {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);

    Serial.println("Sending sensor data...");

    char packet[200];

    sprintf(packet,
            "Acceleration X: %.2f, Y: %.2f, Z: %.2f m/s^2\nRotation X: %.2f, Y: %.2f, Z: %.2f rad/s\nTemperature: %.2f degC\n\n",
            a.acceleration.x,
            a.acceleration.y,
            a.acceleration.z,
            g.gyro.x,
            g.gyro.y,
            g.gyro.z,
            t.temperature);

    radio.write(&packet, sizeof(packet));

    Serial.println("Done");

    delay(500);
}
