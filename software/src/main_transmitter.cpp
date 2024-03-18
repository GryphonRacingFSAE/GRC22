#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <driver/twai.h>
#include <pb_encode.h>

#include "CAN.pb.h"
#include "rlm.h"

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

// CAN
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NO_ACK);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

rlm_rlm_accel_0_xf0_t rlm_accel;
rlm_rlm_gyro_0_xf1_t rlm_gyro;

rlm_rlm_position_0_xf2_t rlm_position;
rlm_rlm_trajectory_0_xf3_t rlm_trajectory;
rlm_rlm_time_0_xf4_t rlm_time;

#define CAN_FRAME_MAX_SIZE 64
uint8_t can_frame[CAN_FRAME_MAX_SIZE];

// MPU-6050
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

// BN-220
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

//==============================================================================
// nRF24L01+
//==============================================================================

void initNRF() {
    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }

    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_MAX);

    radio.openWritingPipe(address);
    radio.stopListening();
}

void sendProto(CAN msg) {
    pb_ostream_t output_stream = pb_ostream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
    pb_encode(&output_stream, CAN_fields, &msg);
    radio.write(nrf_buffer, output_stream.bytes_written);
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
    twai_message_t can_message;

    if (twai_receive(&can_message, pdMS_TO_TICKS(10000)) == ESP_OK) {
        if (!can_message.rtr) {
            CAN msg = CAN_init_default;
            msg.address = can_message.identifier;
            msg.data.size = can_message.data_length_code;
            memcpy(msg.data.bytes, can_message.data, can_message.data_length_code);

            Serial.printf("\e[0;34m[0x%X] \e[0m", msg.address);
            for (int i = 0; i < msg.data.size; i++) {
                Serial.printf("\e[0;34m%02X \e[0m", msg.data.bytes[i]);
            }
            Serial.println();

            sendProto(msg);
        }
    } else {
        Serial.println("Failed to receive CAN message");
    }
}

void sendCAN(uint32_t address, uint8_t* can_frame, int length) {
    twai_message_t can_message;

    can_message.identifier = address;
    can_message.data_length_code = length;
    for (int i = 0; i < length; i++) {
        can_message.data[i] = can_frame[i];
    }

    if (twai_transmit(&can_message, pdMS_TO_TICKS(10000)) == ESP_OK) {
        CAN msg = CAN_init_default;
        msg.address = can_message.identifier;
        msg.data.size = can_message.data_length_code;
        memcpy(msg.data.bytes, can_message.data, can_message.data_length_code);

        Serial.printf("\e[0;32m[0x%X] \e[0m", msg.address);
        for (int i = 0; i < msg.data.size; i++) {
            Serial.printf("\e[0;32m%02X \e[0m", msg.data.bytes[i]);
        }
        Serial.println();

        sendProto(msg);
    } else {
        printf("Failed to send CAN message\n");
    }
}

uint64_t getSeconds() {
    return (gps.time.hour() * 3600 + gps.time.minute() * 60 + gps.time.second());
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

    float ax_real = 4 * ((float)ax / 32768);
    float ay_real = 4 * ((float)ay / 32768);
    float az_real = 4 * ((float)az / 32768);
    float gx_real = 250 * ((float)gx / 32768);
    float gy_real = 250 * ((float)gx / 32768);
    float gz_real = 250 * ((float)gx / 32768);

    // Serial.printf("AX %.3f\tAY %.3f\tAZ %.3f\n", ax_real, ay_real, az_real);
    // Serial.printf("GX %.3f\tGY %.3f\tGZ %.3f\n", gx_real, gy_real, gz_real);

    rlm_accel.x_accel = rlm_rlm_accel_0_xf0_x_accel_encode(ax_real);
    rlm_accel.y_accel = rlm_rlm_accel_0_xf0_y_accel_encode(ay_real);
    rlm_accel.z_accel = rlm_rlm_accel_0_xf0_z_accel_encode(az_real);
    rlm_rlm_accel_0_xf0_pack(can_frame, &rlm_accel, CAN_FRAME_MAX_SIZE);
    sendCAN(RLM_RLM_ACCEL_0_XF0_FRAME_ID, can_frame, RLM_RLM_ACCEL_0_XF0_LENGTH);

    rlm_gyro.x_rot = rlm_rlm_gyro_0_xf1_x_rot_encode(gx_real);
    rlm_gyro.y_rot = rlm_rlm_gyro_0_xf1_y_rot_encode(gy_real);
    rlm_gyro.z_rot = rlm_rlm_gyro_0_xf1_z_rot_encode(gz_real);
    rlm_rlm_gyro_0_xf1_pack(can_frame, &rlm_gyro, CAN_FRAME_MAX_SIZE);
    sendCAN(RLM_RLM_GYRO_0_XF1_FRAME_ID, can_frame, RLM_RLM_GYRO_0_XF1_LENGTH);
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
            // Serial.printf("LAT %f\tLNG %f\n", gps.location.lat(), gps.location.lng());

            rlm_position.latitude = rlm_rlm_position_0_xf2_latitude_encode(gps.location.lat());
            rlm_position.longitude = rlm_rlm_position_0_xf2_longitude_encode(gps.location.lng());
            rlm_rlm_position_0_xf2_pack(can_frame, &rlm_position, CAN_FRAME_MAX_SIZE);
            sendCAN(RLM_RLM_POSITION_0_XF2_FRAME_ID, can_frame, RLM_RLM_POSITION_0_XF2_LENGTH);

            rlm_trajectory.direction = rlm_rlm_trajectory_0_xf3_direction_encode(gps.course.deg());
            rlm_trajectory.speed = rlm_rlm_trajectory_0_xf3_speed_encode(gps.speed.mps());
            rlm_rlm_trajectory_0_xf3_pack(can_frame, &rlm_trajectory, CAN_FRAME_MAX_SIZE);
            sendCAN(RLM_RLM_TRAJECTORY_0_XF3_FRAME_ID, can_frame, RLM_RLM_TRAJECTORY_0_XF3_LENGTH);

            return;
        }
    }
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

uint32_t delta_time;

void loop() {
    delta_time = millis();

    rlm_time.time = delta_time;
    rlm_rlm_time_0_xf4_pack(can_frame, &rlm_time, CAN_FRAME_MAX_SIZE);
    sendCAN(RLM_RLM_TIME_0_XF4_FRAME_ID, can_frame, RLM_RLM_TIME_0_XF4_LENGTH);

    readMPU();
    // readGPS();

    // readCAN();

    /*
    if (digitalRead(MPU_CAL) == HIGH) {
        calibrateMPU();
    }
    */
}
