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

// Constants for pin numbers
// TODO: probably move this to a header file
#define CAN_RX 17
#define CAN_TX 16
#define GPS_RX 33
#define GPS_TX 32
#define MPU_CAL 13
#define NRF_CE 25
#define NRF_CSN 26
#define SD_CS 27
#define SD_OFF 12
#define V_SENSE 15

RF24 radio(NRF_CE, NRF_CSN); // CE, CSN
MPU6050 mpu;
TinyGPSPlus gps;

HardwareSerial SerialGPS(1);

File data_file;
bool do_write = false;
String file_name = "";
String file_out;

const byte address[6] = "00001";

unsigned long int start_time;
unsigned long int delta_time;

int16_t ax, ay, az, ax_offset, ay_offset, az_offset;
int16_t gx, gy, gz, gx_offset, gy_offset, gz_offset;

unsigned long int getSeconds();
void fileInit();
void writeToFile();
void calibrateMPU();
void applyCalibration();

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
    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX); // RX, TX
    Serial.println("Done");

    pinMode(SD_OFF, INPUT);
    pinMode(MPU_CAL, INPUT);

    calibrateMPU();
}

void loop() {

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    applyCalibration();

    while (SerialGPS.available() > 0) {
        if (gps.encode(SerialGPS.read())) {
            break;
        }
    }

#ifdef DEBUG

    if (do_write) {

        Serial.printf("Current Time: %lu\n", delta_time);
        Serial.printf("ACCEL: X %d, Y %d, Z %d\n", ax, ay, az);
        Serial.printf("GYRO:  X %d, Y %d, Z %d\n", gx, gy, gz);
        Serial.printf("GPS:   LAT %.2f, LNG %.2f, ALT %.2f\n\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    }

#endif

    if (start_time == 0) {

        start_time = getSeconds();

        if (start_time != 0) {
            fileInit();
        }
    }

    delta_time = getSeconds() - start_time;

    if (do_write) {

        writeToFile();
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

    if (digitalRead(SD_OFF) == HIGH) {

        if (do_write) {
            data_file = SD.open(file_name, FILE_WRITE);
        } else {
            data_file.close();
        }

        do_write = !do_write;

        delay(150);
    }

    if (digitalRead(MPU_CAL) == HIGH) {

        calibrateMPU();
    }

    delay(100);
}

void calibrateMPU() {

    mpu.getMotion6(&ax_offset, &ay_offset, &az_offset, &gx_offset, &gy_offset, &gz_offset);
    Serial.printf("\n*****MPU Recalibrated*****\n");

    if (data_file) {

        data_file.println("****MPU Recalibrated****");
    }

    delay(150);
}

unsigned long int getSeconds() {
    unsigned long int total_seconds;

    total_seconds = (gps.time.hour() * 3600) + (gps.time.minute() * 60) + (gps.time.second());

    return total_seconds;
}

void applyCalibration() {

    ax -= ax_offset;
    ay -= ay_offset;
    az -= az_offset;
    gx -= gx_offset;
    gy -= gy_offset;
    gz -= gz_offset;
}

void fileInit() {

    // SD Test (D27 on board)

    if (!SD.begin(SD_CS)) {
        Serial.println("Card Mount Failed");
        return;
    } else {

        Serial.println("SD Card Mounted");
    }

    file_name = "/" + String(gps.date.year()) + "_" + String(gps.date.month()) + "_" + String(gps.date.day()) + "_" + String(gps.time.hour()) + "_" +
                String(gps.time.minute()) + "_" + String(gps.time.second()) + ".csv";

    data_file = SD.open(file_name, FILE_WRITE);

    if (data_file.println("DELTA-TIME, XACCEL, YACCEL, ZACCEL, XGYRO, YGYRO, ZGYRO, LAT, LNG, ALT")) {
        Serial.println("File Written");
    } else {
        Serial.println("Write Failed: Check SD Card");
    }

    do_write = !do_write;
}

void writeToFile() {

    data_file.print(delta_time);
    data_file.print(",");
    data_file.print(ax);
    data_file.print(",");
    data_file.print(ay);
    data_file.print(",");
    data_file.print(az);
    data_file.print(",");
    data_file.print(gx);
    data_file.print(",");
    data_file.print(gy);
    data_file.print(",");
    data_file.print(gz);
    data_file.print(",");
    data_file.print(gps.location.lat());
    data_file.print(",");
    data_file.print(gps.location.lng());
    data_file.print(",");
    data_file.print(gps.altitude.meters());
    data_file.print("\n");
}