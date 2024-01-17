#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <TinyGPSPlus.h>
#include <driver/can.h>
#include <pb_encode.h>

// SD Card Libraries
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "message.pb.h"

#define CAN_RX GPIO_NUM_17
#define CAN_TX GPIO_NUM_16
#define GPS_RX 33
#define GPS_TX 32
#define MPU_CAL 13
#define NRF_CE 25
#define NRF_CSN 26
#define SD_CS 27
#define SD_OFF 12
#define V_SENSE 15

//================================================================================
// nRF24L01+
//================================================================================

RF24 radio(NRF_CE, NRF_CSN);
const byte address[6] = "00001";

void initNRF() {
    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }
    radio.openWritingPipe(address);
    radio.stopListening();
}

//================================================================================
// MPU-6050
//================================================================================

MPU6050 mpu;

int16_t ax, ay, az;
int16_t gx, gy, gz;

int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

void initMPU() {
    if (Wire.begin()) {
        Serial.println("I2C initialized successfully");
    } else {
        Serial.println("Failed to initialize I2C");
    }
    mpu.initialize();
}

void calibrateMPU() {
    mpu.getMotion6(&ax_offset, &ay_offset, &az_offset, &gx_offset, &gy_offset, &gz_offset);
    Serial.printf("\nMPU RECALIBRATED\n");

    /*
    if (data_file) {
        data_file.println("MPU RECALIBRATED");
    }
    */

    delay(500);
}

void readMPU() {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    ax -= ax_offset;
    ay -= ay_offset;
    az -= az_offset;
    gx -= gx_offset;
    gy -= gy_offset;
    gz -= gz_offset;

    Serial.printf("AX %d, AY %d, AZ %d\n", ax, ay, az);
    Serial.printf("GX %d, GY %d, GZ %d\n", gx, gy, gz);
}

//================================================================================
// BN-220
//================================================================================

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

void initGPS() {
    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    Serial.println("GPS serial port initialized successfully");
}

void readGPS() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }

    Serial.printf("LAT %.2f, LNG %.2f, ALT %.2f\n\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
}

//================================================================================
// CAN
//================================================================================

can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, CAN_MODE_NORMAL);
can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

can_message_t can_message;

void initCAN() {
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("CAN driver installed successfully");
    } else {
        Serial.println("Failed to install CAN driver");
    }

    if (can_start() == ESP_OK) {
        Serial.println("CAN driver started successfully");
    } else {
        Serial.println("Failed to start CAN driver");
    }
}

// NEED TO MOVE THIS STUFF

File data_file;
bool do_write = false;
String file_name = "";
String file_out;

unsigned long int start_time;
unsigned long int delta_time;

unsigned long int getSeconds();
void fileInit();
void writeToFile();

//================================================================================
// Setup
//================================================================================

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    initNRF();
    initMPU();
    initGPS();
    initCAN();

    pinMode(SD_OFF, INPUT);
    pinMode(MPU_CAL, INPUT);

    calibrateMPU();
}

//================================================================================
// Loop
//================================================================================

void loop() {
    Serial.printf("Current Time: %lu\n", delta_time);

    readGPS();
    readMPU();

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

    if (can_receive(&can_message, pdMS_TO_TICKS(100)) == ESP_OK) {
        Serial.println("CAN message received");

        if (can_message.flags & CAN_MSG_FLAG_EXTD)
            printf("Message is in Extended Format\n");
        else
            printf("Message is in Standard Format\n");

        Serial.printf("ID: %d\n", can_message.identifier);
        if (!(can_message.flags & CAN_MSG_FLAG_RTR)) {
            Serial.printf("Data: ");
            for (int i = 0; i < can_message.data_length_code; i++) {
                printf("%d ", can_message.data[i]);
            }
            Serial.println();
        }
    }
}

unsigned long int getSeconds() {
    unsigned long int total_seconds;

    total_seconds = (gps.time.hour() * 3600) + (gps.time.minute() * 60) + (gps.time.second());

    return total_seconds;
}

void fileInit() {
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
