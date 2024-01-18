#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <driver/can.h>
#include <pb_encode.h>

#include "message.pb.h"

//================================================================================
// Global
//================================================================================

// GPIO
#define SD_OFF GPIO_NUM_12
#define MPU_CAL GPIO_NUM_13
#define V_SENSE GPIO_NUM_15
#define CAN_TX GPIO_NUM_16
#define CAN_RX GPIO_NUM_17
#define NRF_CE GPIO_NUM_25
#define NRF_CSN GPIO_NUM_26
#define SD_CS GPIO_NUM_27
#define GPS_TX GPIO_NUM_32
#define GPS_RX GPIO_NUM_33

// nRF24L01+
RF24 radio(NRF_CE, NRF_CSN);
const byte address[6] = "00001";

// MPU-6050
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

// BN-220
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

// CAN
can_message_t can_message;
can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, CAN_MODE_NORMAL);
can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

// SD Card
File csv_file;
bool is_writing;
uint64_t start_time;
uint64_t delta_time;

//================================================================================
// nRF24L01+
//================================================================================

void initNRF() {
    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }
    radio.openWritingPipe(address);
    radio.stopListening();
}

void sendMessage() {}

//================================================================================
// MPU-6050
//================================================================================

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

    if (csv_file) {
        csv_file.println("MPU RECALIBRATED");
    }

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

    Serial.printf("AX %d | AY %d | AZ %d\n", ax, ay, az);
    Serial.printf("GX %d | GY %d | GZ %d\n", gx, gy, gz);
}

//================================================================================
// BN-220
//================================================================================

void initGPS() {
    SerialGPS.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
    Serial.println("GPS serial port initialized successfully");
}

void readGPS() {
    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }

    Serial.printf("LAT %.2f | LNG %.2f | ALT %.2f\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
}

//================================================================================
// CAN
//================================================================================

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

void readCAN() {
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
    } else {
        Serial.println("Failed to receive CAN message");
    }
}

//================================================================================
// SD Card
//================================================================================

void initFile() {
    if (SD.begin(SD_CS)) {
        Serial.println("SD initialized successfully");
    } else {
        Serial.println("Failed to initialize SD");
    }

    char file_name[32];
    sprintf(file_name, "/%d-%d-%d_%d-%d-%d.csv", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());

    csv_file = SD.open(file_name, FILE_WRITE);
    if (csv_file) {
        Serial.printf("CSV file created successfully: %s\n", file_name);
    } else {
        Serial.println("Failed to create CSV file");
    }

    is_writing = true;

    csv_file.println("TIME,AX,AY,AZ,GX,GY,GZ,LAT,LNG,ALT");

    delay(500);
}

void stopWriting() {
    if (is_writing) {
        csv_file.close();
        is_writing = false;
        Serial.println("SD writing disabled");
    } else {
        Serial.println("SD writing is not enabled");
    }

    delay(500);
}

uint64_t getCentiseconds() {
    return (gps.time.hour() * 360000 + gps.time.minute() * 6000 + gps.time.second() * 100 + gps.time.centisecond());
}

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
    if (start_time == 0) {
        start_time = getCentiseconds();

        if (start_time != 0) {
            initFile();
        }
    }

    delta_time = getCentiseconds() - start_time;
    Serial.printf("TIME %lu\n", delta_time);

    readMPU();
    readGPS();

    if (is_writing) {
        csv_file.printf("%lu,%hd,%hd,%hd,%hd,%hd,%hd,%f,%f,%f\n", delta_time, ax, ay, az, gx, gy, gz, gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    }

    if (digitalRead(MPU_CAL) == HIGH) {
        calibrateMPU();
    }
    if (digitalRead(SD_OFF) == HIGH) {
        stopWriting();
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

    readCAN();

    Serial.println();
}
