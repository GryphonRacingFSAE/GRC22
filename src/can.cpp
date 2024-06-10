#include <HardwareSerial.h>
#include <driver/twai.h>

#include "can.h"
#include "globals.h"
#include "utils.h"

// CAN
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

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

void startTransmitCANTask(void* pvParameters) {
    (void)pvParameters;

    twai_message_t tx_msg = {};

    // Clear motor controller faults
    tx_msg.identifier = 0x0c1;
    tx_msg.data_length_code = 8;

    tx_msg.data[0] = 20;
    tx_msg.data[1] = 0;
    tx_msg.data[2] = 1;
    tx_msg.data[3] = 0;
    tx_msg.data[4] = 0;
    tx_msg.data[5] = 0;
    tx_msg.data[6] = 0;
    tx_msg.data[7] = 0;

    auto resp = twai_transmit(&tx_msg, pdMS_TO_TICKS(1));
    if (resp != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }

    TickType_t tick = xTaskGetTickCount();

    while (1) {
        // Send messages that should be transmitted every 3ms
        if (tick % 3 == 0) {
            sendTorque();
        }

        // Send messages that should be transmitted every 100ms
        if (tick % 100 == 0) {
            sendState();
            sendPedals();
            sendIMD();
            sendFlowSensor();
        }

        // Send messages that should be transmitted every 1000ms
        if (tick % 1000 == 0) {
            sendTorqueParameters();
            sendAmsDtcMask();
        }

        xTaskDelayUntil(&tick, pdMS_TO_TICKS(1));
    }
}

void startReceiveCANTask(void* pvParameters) {
    (void)pvParameters;
    twai_message_t rx_msg;

    while (1) {
        if (twai_receive(&rx_msg, portMAX_DELAY) == ESP_OK) {
            switch (rx_msg.identifier) {
            case 0x0A2: { // INV_Hot_Spot_Temp, INV_Coolant_Temp
                uint16_t INV_Hot_Spot_Temp = ((uint16_t)rx_msg.data[3] << 8) | ((uint16_t)rx_msg.data[2]);
                global_motor_controller.motor_controller_temp = *(int16_t*)(&INV_Hot_Spot_Temp);
                uint16_t INV_Coolant_Temp = ((uint16_t)rx_msg.data[1] << 8) | ((uint16_t)rx_msg.data[0]);
                global_motor_controller.coolant_temp = *(int16_t*)(&INV_Coolant_Temp);
                break;
            }
            case 0x0A7: { // INV_DC_Bus_Voltage
                uint16_t INV_DC_Bus_Voltage = ((uint16_t)rx_msg.data[1] << 8) | ((uint16_t)rx_msg.data[0]);
                global_motor_controller.tractive_voltage = *(int16_t*)(&INV_DC_Bus_Voltage);
                break;
            }
            case 0x0B0: { // INV_Motor_Speed
                uint16_t INV_Motor_Speed = ((uint16_t)rx_msg.data[3] << 8) | ((uint16_t)rx_msg.data[2]);
                global_motor_controller.motor_speed = *(int16_t*)(&INV_Motor_Speed);
                break;
            }
            case 0x0E0: { // Custom BMS MSG
                uint8_t max_temp_raw = (uint8_t)(rx_msg.data[0]);
                global_bms.max_temp = *(int8_t*)(&max_temp_raw);
                break;
            }
            case 0x0E5: {
                global_bms.DTC1 = (rx_msg.data[1] << 8) | rx_msg.data[0];
                global_bms.DTC2 = (rx_msg.data[3] << 8) | rx_msg.data[2];
                global_bms.last_heartbeat = xTaskGetTickCount();
                break;
            }
            case 0x400: {
                // Torque Parameter Editing
                uint16_t torque_raw = ((uint16_t)rx_msg.data[1] << 8) | ((uint16_t)rx_msg.data[0]);
                int16_t new_torque_raw = *(int16_t*)(&torque_raw);

                uint16_t power_raw = ((uint16_t)rx_msg.data[3] << 8) | ((uint16_t)rx_msg.data[2]);
                int16_t new_power_raw = *(int16_t*)(&power_raw);

                uint16_t target_speed_limit = ((uint16_t)rx_msg.data[5] << 8) | ((uint16_t)rx_msg.data[4]);
                int16_t new_target_speed_limit = *(int16_t*)(&target_speed_limit);
                uint8_t idle_pump_raw = rx_msg.data[6];

                // update stored values if different
                param_storage.putShort("torqueRaw", new_torque_raw);
                param_storage.putShort("powerRaw", new_power_raw);
                param_storage.putShort("targetSpeedLim", new_target_speed_limit);
                param_storage.putUChar("idlePumpSpeed", idle_pump_raw);

                break;
            }
            case 0x401: {
               uint16_t dtc1_mask = ((uint16_t)rx_msg.data[1] << 8) | ((uint16_t)rx_msg.data[0]);
               uint16_t dtc2_mask = ((uint16_t)rx_msg.data[3] << 8) | ((uint16_t)rx_msg.data[2]);

               param_storage.putShort("dtc1Mask", dtc1_mask);
               param_storage.putShort("dtc2Mask", dtc2_mask);
               break;
            }
            }
        }
    }
}

void sendTorque() {
    twai_message_t tx_msg = {};

    tx_msg.identifier = 0x0c0;
    tx_msg.data_length_code = 8;

    if (FLAG_ACTIVE(global_output_peripherals.flags, CTRL_RTD_INVALID)) {
        for (uint8_t i = 0; i < 8; i++) {
            tx_msg.data[i] = 0;
        }
    } else {
        // Bytes 0 & 1 is the requested torque
        uint16_t bitwise_requested_torque = 0;
        if (!FAULTS_ACTIVE(global_output_peripherals.flags)) {
            bitwise_requested_torque = *(uint16_t*)&global_output_peripherals.requested_torque;
        }
        tx_msg.data[0] = bitwise_requested_torque & 0xFF;
        tx_msg.data[1] = bitwise_requested_torque >> 8;

        // Bytes 2 & 3 is the requested RPM (if not in torque mode)
        tx_msg.data[2] = 0;
        tx_msg.data[3] = 0;

        // Byte 4 is Forward/Reverse
        tx_msg.data[4] = 0; // 1 is Forward

        // Byte 5 is Configuration
        tx_msg.data[5] = 0x1; // Inverter Enable

        // Byte 6 & 7 sets torque limits
        tx_msg.data[6] = 0;
        tx_msg.data[7] = 0;
    }

    auto resp = twai_transmit(&tx_msg, pdMS_TO_TICKS(1));
    if (resp != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendState() {
    // Format is defined in VCU.dbc
    twai_message_t tx_msg;
    tx_msg.identifier = 0x300;
    tx_msg.data_length_code = 2;

    uint16_t flags = global_output_peripherals.flags;
    tx_msg.data[0] = flags & 0xFF;
    tx_msg.data[1] = (flags >> 8) && 0xFF;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendPedals() {
    // Format is defined in VCU.dbc
    twai_message_t tx_msg;
    tx_msg.identifier = 0x301;
    tx_msg.data_length_code = 4;

    uint16_t position = global_peripherals.pedal_position;
    tx_msg.data[0] = position & 0xFF;
    tx_msg.data[1] = position >> 8;

    uint16_t pressure = global_peripherals.brake_pressure;
    tx_msg.data[2] = pressure & 0xFF;
    tx_msg.data[3] = pressure >> 8;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendIMD() {
    twai_message_t tx_msg;
    tx_msg.identifier = 0x304;
    tx_msg.data_length_code = 3;

    tx_msg.data[0] = global_imd.state;

    uint16_t resistance = global_imd.resistance;
    tx_msg.data[1] = resistance & 0xFF;
    tx_msg.data[2] = resistance >> 8;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendTorqueParameters() {
    twai_message_t tx_msg;
    tx_msg.identifier = 0x305;
    tx_msg.data_length_code = 7;

    int16_t torque = param_storage.getShort("torqueRaw", DEFAULT_TORQUE);
    tx_msg.data[0] = torque & 0xFF;
    tx_msg.data[1] = torque >> 8;

    int16_t power = param_storage.getShort("powerRaw", DEFAULT_POWER);
    tx_msg.data[2] = power & 0xFF;
    tx_msg.data[3] = power >> 8;

    int16_t target_speed = param_storage.getShort("targetSpeedLim", DEFAULT_TARGET_SPEED_LIM);
    tx_msg.data[4] = target_speed & 0xFF;
    tx_msg.data[5] = target_speed >> 8;

    uint8_t idle_pump_speed = param_storage.getUChar("idlePumpSpeed", DEFAULT_IDLE_PUMP_SPEED);
    tx_msg.data[6] = idle_pump_speed & 0xFF;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendFlowSensor() {
    twai_message_t tx_msg;
    tx_msg.identifier = 0x306;
    tx_msg.data_length_code = 2;

    uint16_t flow_rate = global_peripherals.flow_rate;
    tx_msg.data[0] = flow_rate & 0xFF;
    tx_msg.data[1] = (flow_rate >> 8) && 0xFF;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}

void sendAmsDtcMask(){
    twai_message_t tx_msg;
    tx_msg.identifier = 0x307;
    tx_msg.data_length_code = 4;

    uint16_t dtc1_mask = param_storage.getShort("dtc1Mask", DEFAULT_DTC1_MASK);
    tx_msg.data[0] = dtc1_mask & 0xFF;
    tx_msg.data[1] = (dtc1_mask >> 8) && 0xFF;

    uint16_t dtc2_mask = param_storage.getShort("dtc2Mask", DEFAULT_DTC2_MASK);
    tx_msg.data[2] = dtc2_mask & 0xFF;
    tx_msg.data[3] = (dtc2_mask >> 8) && 0xFF;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1)) != ESP_OK) {
        Serial.printf("Failed to send CAN message: 0x%0x\n", tx_msg.identifier);
    }
}
