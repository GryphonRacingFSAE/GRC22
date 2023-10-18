#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <TinyGPSPlus.h>
#include <pb_encode.h>

// SD Card Libraries
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "message.pb.h"

#define DEBUG

RF24 radio(4, 5); // CE, CSN
MPU6050 mpu;
TinyGPSPlus gps;

HardwareSerial SerialGPS(1);

File dataFile;
bool doWrite = false;
String fileName = "";

const byte address[6] = "00001";

unsigned long int startTime;
unsigned long int deltaTime;

unsigned long int getSeconds() {
    unsigned long int totalSeconds;

    totalSeconds = (gps.time.hour() * 3600) + (gps.time.minute() * 60) + (gps.time.second());

    return totalSeconds;
}

void fileInit() {

    // SD Test (D27 on board)

    if (!SD.begin(27)) {
        Serial.println("Card Mount Failed");
        return;
    } else {

        Serial.println("SD Card Mounted");
    }

    fileName = "/" + String(gps.date.year()) + "_" + String(gps.date.month()) + "_" + String(gps.date.day()) + "_" + String(gps.time.hour()) + "_" +
               String(gps.time.minute()) + "_" + String(gps.time.second()) + ".csv";

    dataFile = SD.open(fileName, FILE_WRITE);

    if (dataFile.println("DELTA-TIME, XACCEL, YACCEL, ZACCEL, XGYRO, YGYRO, ZGYRO, LAT, LNG, ALT")) {
        Serial.println("File Written");
    } else {
        Serial.println("Write Failed: Check SD Card");
    }

    doWrite = !doWrite;
}

void setup() {
    Serial.begin(115200);
    delay(500);

    while (!Serial)
        delay(100);

    Serial.println("\nInitializing nRF24L01+...");
    radio.begin();
    radio.openWritingPipe(address);
    radio.stopListening();
    Serial.println("Done");

    Serial.println("Initializing MPU-6050...");
    Wire.begin();
    mpu.initialize();
    Serial.println("Done");

    Serial.println("Initializing BN-220...");
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
    Serial.println("Done");

    pinMode(35, INPUT);
}

int16_t ax, ay, az;
int16_t gx, gy, gz;

void loop() {

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    while (SerialGPS.available() > 0) {
        if (gps.encode(SerialGPS.read())) {
            break;
        }
    }

#ifdef DEBUG

    if (doWrite) {

        Serial.printf("Current Time: %lu\n", deltaTime);
        Serial.printf("ACCEL: X %d, Y %d, Z %d\n", ax, ay, az);
        Serial.printf("GYRO:  X %d, Y %d, Z %d\n", gx, gy, gz);
        Serial.printf("GPS:   LAT %.2f, LNG %.2f, ALT %.2f\n\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    }

#endif

    if (startTime == 0) {

        startTime = getSeconds();

        if (startTime != 0) {
            fileInit();
        }
    }

    deltaTime = getSeconds() - startTime;

    if (doWrite) {

        dataFile.print(deltaTime);
        dataFile.print(",");
        dataFile.print(ax);
        dataFile.print(",");
        dataFile.print(ay);
        dataFile.print(",");
        dataFile.print(az);
        dataFile.print(",");
        dataFile.print(gx);
        dataFile.print(",");
        dataFile.print(gy);
        dataFile.print(",");
        dataFile.print(gz);
        dataFile.print(",");
        dataFile.print(gps.location.lat());
        dataFile.print(",");
        dataFile.print(gps.location.lng());
        dataFile.print(",");
        dataFile.print(gps.altitude.meters());
        dataFile.print("\n");
    }

    MyMessage msg = MyMessage_init_default;

    // Values are opposite for some reason (but it works)
    msg.acceleration_x = ax;
    msg.acceleration_y = ay;
    msg.acceleration_z = az;
    msg.rotation_x = gx;
    msg.rotation_y = gy;
    msg.rotation_z = gz;
    msg.latitude = gps.location.lat();
    msg.longitude = gps.location.lng();
    msg.altitude = gps.altitude.meters();

    uint8_t buffer[128];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&stream, MyMessage_fields, &msg);

    radio.write(buffer, stream.bytes_written);

    if (digitalRead(35) == HIGH) {

        if (doWrite) {
            dataFile = SD.open(fileName, FILE_WRITE);
        } else {
            dataFile.close();
        }

        doWrite = !doWrite;

        delay(150);
    }

    delay(100);
}
