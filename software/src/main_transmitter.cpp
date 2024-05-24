#include <Arduino.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <RF24.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>
#include <pb_encode.h>

#include "message.pb.h"
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

#define CAN_MAX_SIZE 64
uint8_t can_frame[CAN_MAX_SIZE];

// MPU-6050
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t ax_offset, ay_offset, az_offset;
int16_t gx_offset, gy_offset, gz_offset;

// BN-220
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

// FreeRTOS
QueueHandle_t can_queue;
QueueHandle_t radio_queue;

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
    radio.setAutoAck(false);
    radio.setPALevel(RF24_PA_MAX);

    radio.openWritingPipe(address);
    radio.stopListening();
}

void sendProto(twai_message_t can_message) {
    ProtoMessage msg = ProtoMessage_init_default;

    msg.address = can_message.identifier;
    msg.data.size = can_message.data_length_code;
    memcpy(msg.data.bytes, can_message.data, msg.data.size);
    msg.time = millis();

    pb_ostream_t output_stream = pb_ostream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
    pb_encode(&output_stream, ProtoMessage_fields, &msg);

    // As auto-ack is disabled, this will always succeed
    if (!radio.write(nrf_buffer, output_stream.bytes_written)) {
        Serial.println("Failed to send radio message");
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
    float gy_real = 250 * ((float)gy / 32768);
    float gz_real = 250 * ((float)gz / 32768);

    rlm_accel.x_accel = rlm_rlm_accel_0_xf0_x_accel_encode(ax_real);
    rlm_accel.y_accel = rlm_rlm_accel_0_xf0_y_accel_encode(ay_real);
    rlm_accel.z_accel = rlm_rlm_accel_0_xf0_z_accel_encode(az_real);
    rlm_rlm_accel_0_xf0_pack(can_frame, &rlm_accel, CAN_MAX_SIZE);

    twai_message_t accel_msg{.identifier = RLM_RLM_ACCEL_0_XF0_FRAME_ID, .data_length_code = RLM_RLM_ACCEL_0_XF0_LENGTH};
    for (int i = 0; i < accel_msg.data_length_code; i++) {
        accel_msg.data[i] = can_frame[i];
    }

    rlm_gyro.x_rot = rlm_rlm_gyro_0_xf1_x_rot_encode(gx_real);
    rlm_gyro.y_rot = rlm_rlm_gyro_0_xf1_y_rot_encode(gy_real);
    rlm_gyro.z_rot = rlm_rlm_gyro_0_xf1_z_rot_encode(gz_real);
    rlm_rlm_gyro_0_xf1_pack(can_frame, &rlm_gyro, CAN_MAX_SIZE);

    twai_message_t gyro_msg{.identifier = RLM_RLM_GYRO_0_XF1_FRAME_ID, .data_length_code = RLM_RLM_GYRO_0_XF1_LENGTH};
    for (int i = 0; i < gyro_msg.data_length_code; i++) {
        gyro_msg.data[i] = can_frame[i];
    }

    xQueueSend(can_queue, &accel_msg, portMAX_DELAY);
    xQueueSend(can_queue, &gyro_msg, portMAX_DELAY);

    xQueueSend(radio_queue, &accel_msg, portMAX_DELAY);
    xQueueSend(radio_queue, &gyro_msg, portMAX_DELAY);
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
            rlm_position.latitude = rlm_rlm_position_0_xf2_latitude_encode(gps.location.lat());
            rlm_position.longitude = rlm_rlm_position_0_xf2_longitude_encode(gps.location.lng());
            rlm_rlm_position_0_xf2_pack(can_frame, &rlm_position, CAN_MAX_SIZE);

            twai_message_t position_msg{.identifier = RLM_RLM_POSITION_0_XF2_FRAME_ID, .data_length_code = RLM_RLM_POSITION_0_XF2_LENGTH};
            for (int i = 0; i < position_msg.data_length_code; i++) {
                position_msg.data[i] = can_frame[i];
            }

            rlm_trajectory.direction = rlm_rlm_trajectory_0_xf3_direction_encode(gps.course.deg());
            rlm_trajectory.speed = rlm_rlm_trajectory_0_xf3_speed_encode(gps.speed.mps());
            rlm_rlm_trajectory_0_xf3_pack(can_frame, &rlm_trajectory, CAN_MAX_SIZE);

            twai_message_t trajectory_msg{.identifier = RLM_RLM_POSITION_0_XF2_FRAME_ID, .data_length_code = RLM_RLM_POSITION_0_XF2_LENGTH};
            for (int i = 0; i < trajectory_msg.data_length_code; i++) {
                trajectory_msg.data[i] = can_frame[i];
            }

            // xQueueSend(can_queue, &position_msg, 0);
            // xQueueSend(can_queue, &trajectory_msg, 0);

            xQueueSend(radio_queue, &position_msg, 0);
            xQueueSend(radio_queue, &trajectory_msg, 0);
            return;
        }
    }
}

//==============================================================================
// Tasks
//==============================================================================

void canTransmitTask(void* parameter) {
    twai_message_t can_message;
    for (;;) {
        if (xQueueReceive(can_queue, &can_message, portMAX_DELAY) == pdPASS) {
            if (twai_transmit(&can_message, 1) != ESP_OK) {
                printf("Failed to send CAN message\n");
            }
        }
    }
}

void canReceiveTask(void* parameter) {
    twai_message_t can_message;
    for (;;) {
        if (twai_receive(&can_message, portMAX_DELAY) == ESP_OK) {
            xQueueSend(radio_queue, &can_message, 0);
        } else {
            printf("Failed to receive CAN message\n");
        }
    }
}

void sensorReadTask(void* parameter) {
    for (;;) {
        // readMPU();
        readGPS();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void radioTransmitTask(void* parameter) {
    twai_message_t can_message;
    for (;;) {
        if (xQueueReceive(radio_queue, &can_message, portMAX_DELAY) == pdPASS) {
            sendProto(can_message);
        }
    }
}

//==============================================================================
// Setup
//==============================================================================

void setup() {
    Serial.begin(921600);

    initNRF();
    // initMPU();
    initGPS();
    initCAN();

    // calibrateMPU();

    can_queue = xQueueCreate(50, sizeof(twai_message_t));
    radio_queue = xQueueCreate(50, sizeof(twai_message_t));

    // isolate CAN transmission and reception to Core 0
    xTaskCreatePinnedToCore(canTransmitTask, "CAN Transmit Task", 8096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(canReceiveTask, "CAN Receive Task", 8096, NULL, 3, NULL, 0);

    // isolate sensor reading and radio transmission to Core 1
    xTaskCreatePinnedToCore(sensorReadTask, "CAN Receive Task", 8096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(radioTransmitTask, "CAN Receive Task", 8096, NULL, 4, NULL, 1);
}

//==============================================================================
// Loop
//==============================================================================

void loop() {}
