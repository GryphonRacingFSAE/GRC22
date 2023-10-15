#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <TinyGPSPlus.h>
#include <pb_encode.h>

//SD Card Libraries
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#include "message.pb.h"

#define DEBUG

RF24 radio(4, 5); // CE, CSN
MPU6050 mpu;
TinyGPSPlus gps;

HardwareSerial SerialGPS(1);

File testFile;
bool doWrite = true;

const byte address[6] = "00001";

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

    //SD Test (D32 on board)

    if(!SD.begin(27)) {
        Serial.println("Card Mount Failed");
        return;
    }
    else {

        Serial.println("SD Card Mounted");
    }

    uint8_t cardType = SD.cardType();


    testFile = SD.open("/test.csv", FILE_WRITE);

    if(testFile.println("XACCEL, YACCEL, ZACCEL, XGYRO, YGYRO, ZGYRO, LAT, LNG, ALT")){
        Serial.println("File Written");
    }
    else{
        Serial.println("Write Failed: Check SD Card");
    }

    pinMode(35,INPUT);
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

if(doWrite) {
    Serial.printf("ACCEL: X %d, Y %d, Z %d\n", ax, ay, az);
    Serial.printf("GYRO:  X %d, Y %d, Z %d\n", gx, gy, gz);
    Serial.printf("GPS:   LAT %.2f, LNG %.2f, ALT %.2f\n\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());

    testFile.print(ax);
    testFile.print(",");
    testFile.print(ay);
    testFile.print(",");
    testFile.print(az);
    testFile.print(",");
    testFile.print(gx);
    testFile.print(",");
    testFile.print(gy);
    testFile.print(",");
    testFile.print(gz);
    testFile.print(",");
    testFile.print(gps.location.lat());
    testFile.print(",");
    testFile.print(gps.location.lng());
    testFile.print(",");
    testFile.print(gps.altitude.meters());
    testFile.print("\n");
    }

#endif

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

    if(digitalRead(35) == HIGH){

        if(doWrite) {
            testFile = SD.open("/test.csv", FILE_WRITE);
        }
        else {
            testFile.close();
        }

        doWrite = !doWrite;

        delay(150);
    }

    delay(100);
}
