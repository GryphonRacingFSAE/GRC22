#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <driver/twai.h>
#include <pb_encode.h>

#include "CAN.pb.h"

//==============================================================================
// Global
//==============================================================================

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
uint8_t nrf_buffer[128];

// MPU-6050
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

// BN-220
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
uint64_t start_time;
uint64_t delta_time;

// CAN
twai_message_t can_message;
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

//==============================================================================
// nRF24L01+
//==============================================================================

void initNRF() {
    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }
    radio.openWritingPipe(address);
    radio.stopListening();
}

void sendProto(CAN msg) {
    pb_ostream_t output_stream = pb_ostream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
    pb_encode(&output_stream, CAN_fields, &msg);
    radio.write(nrf_buffer, output_stream.bytes_written);
}

//==============================================================================
// MPU-6050
//==============================================================================

void initMPU() {
    if (Wire.begin()) {
        Serial.println("I2C initialized successfully");
    } else {
        Serial.println("Failed to initialize I2C");
    }
    mpu.initialize();
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
}

void calibrateMPU() {
    mpu.getMotion6(&ax_offset, &ay_offset, &az_offset, &gx_offset, &gy_offset, &gz_offset);
    Serial.printf("\nMPU RECALIBRATED\n");
    delay(1000);
}

void readMPU() {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    ax -= ax_offset;
    ay -= ay_offset;
    az -= az_offset;
    gx -= gx_offset;
    gy -= gy_offset;
    gz -= gz_offset;

    Serial.printf("AX %.3f\tAY %.3f\tAZ %.3f\n", 4 * (float(ax) / 32768), 4 * (float(ay) / 32768), 4 * (float(az) / 32768));
    Serial.printf("GX %.3f\tGY %.3f\tGZ %.3f\n", 250 * (float(gx) / 32768), 250 * (float(gy) / 32768), 250 * (float(gz) / 32768));
}

//==============================================================================
// BN-220
//==============================================================================

void initGPS() {
    SerialGPS.begin(115200, SERIAL_8N1, GPS_TX, GPS_RX);
    Serial.println("GPS serial initialized successfully");
}

void readGPS() {
    while (SerialGPS.available() > 0) {
        if (gps.encode(SerialGPS.read())) {
            Serial.printf("LAT %f\tLNG %f\tALT %.1f\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters());

            /*
            RLM_POSITION_0XF2_t* RLM_POSITION;
            RLM_POSITION->LATITUDE_ro = gps.location.lat();
            RLM_POSITION->LONGITUDE_ro = gps.location.lng();

            can_message.identifier =
                Pack_RLM_POSITION_0XF2_dbc_to_cpp(RLM_POSITION, can_message.data, &can_message.data_length_code, RLM_POSITION_0XF2_IDE);

            Serial.printf("ID: %d\n", can_message.identifier);
            Serial.printf("DATA: ");
            for (int i = 0; i < can_message.data_length_code; i++) {
                printf("%d ", can_message.data[i]);
            }
            Serial.println();
            */
            return;
        }
    }
}

//==============================================================================
// CAN
//==============================================================================

void initCAN() {
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("CAN driver installed successfully");
    } else {
        Serial.println("Failed to install CAN driver");
    }

    if (twai_start() == ESP_OK) {
        Serial.println("CAN driver started successfully");
    } else {
        Serial.println("Failed to start CAN driver");
    }
}

void readCAN() {
    if (twai_receive(&can_message, pdMS_TO_TICKS(10000)) == ESP_OK) {
        if (!can_message.rtr) {
            CAN msg = CAN_init_default;
            msg.address = can_message.identifier;
            msg.data.size = can_message.data_length_code;
            memcpy(msg.data.bytes, can_message.data, can_message.data_length_code);

            Serial.printf("[0x%X] ", msg.address);
            for (int i = 0; i < msg.data.size; i++) {
                Serial.printf("%02X ", msg.data.bytes[i]);
            }
            Serial.println();

            sendProto(msg);
        }
    } else {
        Serial.println("Failed to receive CAN message");
    }
}

uint64_t getSeconds() {
    return (gps.time.hour() * 3600 + gps.time.minute() * 60 + gps.time.second());
}

//==============================================================================
// Setup
//==============================================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();

    initNRF();
    initMPU();
    initGPS();
    initCAN();

    pinMode(MPU_CAL, INPUT);

    calibrateMPU();

    Serial.println();
}

//==============================================================================
// Loop
//==============================================================================

void loop() {
    /*
    if (start_time == 0) {
        start_time = getSeconds();
    }

    delta_time = getSeconds() - start_time;

    Serial.printf("TIME %lu\n", delta_time);
    Serial.printf("SATs %d\n", gps.satellites.value());

    readMPU();
    readGPS();
    */

    readCAN();

    /*
    if (digitalRead(MPU_CAL) == HIGH) {
        calibrateMPU();
    }
    */
}
