#include <CAN.h>
#include "utils.h"
#include "control.h"
#include "APPS.h"
#include <string.h>

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

// This task handles scheduling all of the different CAN messages that should be transmitted
void startCANTransmitTask() {
	uint32_t tick = osKernelGetTickCount();

	while (1) {
		// Send messages that should be transmitted every 3ms
		if (tick % 3 == 0) {
			sendTorque();
		}

		// Send messages that should be transmitted every 100ms
		if (tick % 100 == 0) {
			sendState();
			sendPedals();
		}

		osDelayUntil(tick += CAN_TRANSMIT_PERIOD);
	}
}

void startCANTxTask() {
	CANTXMsg tx_msg;

	while (1) {
		// Grab CAN message from CAN1 queue
		if (osMessageQueueGet(CANTX_QHandle, &tx_msg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			TRACE_PRINT("CAN1 sending message: %d or %d\r\n", tx_msg.header.StdId, tx_msg.header.ExtId);
			if (HAL_CAN_AddTxMessage(tx_msg.to, &tx_msg.header, tx_msg.data, NULL) != HAL_OK) {
				ERROR_PRINT("Could not transmit on CAN!\r\n");
			}
		}
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CANRXMsg rxMsg = { .from = hcan }; // Allow the message handler to know where to send any responses to.
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxMsg.header, rxMsg.data);
	osMessageQueuePut(CANRX_QHandle, &rxMsg, 0, 0);
	TRACE_PRINT("CAN received message: %d or %d\r\n", rxMsg.header.StdId, rxMsg.header.ExtId);
}

// INFO: Because we only have one task for receiving messages from CAN, all CAN inputs can be considered "serial"
void startCANRxTask() {
	CANRXMsg rx_msg;

	while (1) {
		// Grab CAN message from RX queue
		if (osMessageQueueGet(CANRX_QHandle, &rx_msg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			DEBUG_PRINT("CAN receiving message: %d or %d\r\n", rx_msg.header.StdId, rx_msg.header.ExtId);
			canMsgHandler(&rx_msg);
		}
	}
}

inline void canMsgHandler(CANRXMsg* rxMsg) {
	DEBUG_PRINT("CAN receiving message: %d or %d\n", rxMsg->header.StdId, rxMsg->header.ExtId);
	switch (rxMsg->header.StdId) {
	case 0x0A2: { //INV_Hot_Spot_Temp, INV_Coolant_Temp
		uint16_t INV_Hot_Spot_Temp = ((uint16_t) rxMsg->data[3] << 8) | ((uint16_t) rxMsg->data[2]);
		Ctrl_Data.motor_controller_temp = *(int16_t*) (&INV_Hot_Spot_Temp);
		uint16_t INV_Coolant_Temp = ((uint16_t) rxMsg->data[1] << 8) | ((uint16_t) rxMsg->data[0]);
		Ctrl_Data.coolant_temp = *(int16_t*) (&INV_Coolant_Temp);
		break;
	}
	case 0x0A7: { // INV_DC_Bus_Voltage
		uint16_t INV_DC_Bus_Voltage = ((uint16_t) rxMsg->data[1] << 8) | ((uint16_t) rxMsg->data[0]);
		Ctrl_Data.tractive_voltage = *(int16_t*) (&INV_DC_Bus_Voltage);
		break;
	}
	case 0x0A5: { // INV_Motor_Speed
		uint16_t INV_Motor_Speed = ((uint16_t) rxMsg->data[3] << 8) | ((uint16_t) rxMsg->data[2]);
		Ctrl_Data.motor_speed = *(int16_t*) (&INV_Motor_Speed);
		break;
	}
	}
}

void sendTorque() {
	// Format is defined in CM200DZ CAN protocol V6.1 section 2.2
	CANTXMsg tx_msg;
	tx_msg.header.IDE = CAN_ID_STD;
	tx_msg.header.RTR = CAN_RTR_DATA;
	tx_msg.header.StdId = 0x0C0;
	tx_msg.header.DLC = 8;
	tx_msg.to = &hcan2;

	if (FLAG_ACTIVE(Ctrl_Data.flags, CTRL_RTD_INVALID)) {
		for (uint8_t i = 0; i < 8; i++){
			tx_msg.data[i] = 0;
		}
	} else {
		// Bytes 0 & 1 is the requested torque
		uint16_t bitwise_requested_torque = 0;
		if (!APPS_Data.flags) {
			bitwise_requested_torque = *(uint16_t*)&APPS_Data.torque;
		}
		tx_msg.data[0] = bitwise_requested_torque & 0xFF;
		tx_msg.data[1] = bitwise_requested_torque >> 8;

		// Bytes 2 & 3 is the requested RPM (if not in torque mode)
		tx_msg.data[2] = 0;
		tx_msg.data[3] = 0;

		// Byte 4 is Forward/Reverse
		tx_msg.data[4] = 1; // 1 is Forward

		// Byte 5 is Configuration
		tx_msg.data[5] = 0x1;
			// | 0x1 // Inverter Enable
			// | 0x2 // Inverter Discharge
			// | 0x4 // Speed Mode override

		// Byte 6 & 7 sets torque limits
		tx_msg.data[6] = 0;
		tx_msg.data[7] = 0;
	}


	// Send over CAN2
	osMessageQueuePut(CANTX_QHandle, &tx_msg, 0, 5);
}

void sendState() {
	// Format is defined in VCU.dbc
	CANTXMsg tx_msg;
	tx_msg.header.IDE = CAN_ID_STD;
	tx_msg.header.RTR = CAN_RTR_DATA;
	tx_msg.header.StdId = 0x200;
	tx_msg.header.DLC = 2;
	tx_msg.to = &hcan2;

	uint16_t flags = (Ctrl_Data.flags << 4) | APPS_Data.flags;
	tx_msg.data[0] = flags & 0xFF;
	tx_msg.data[1] = flags >> 8;

	// Send over CAN2
	osMessageQueuePut(CANTX_QHandle, &tx_msg, 0, 5);
}

void sendPedals() {
	// Format is defined in VCU.dbc
	CANTXMsg tx_msg;
	tx_msg.header.IDE = CAN_ID_STD;
	tx_msg.header.RTR = CAN_RTR_DATA;
	tx_msg.header.StdId = 0x201;
	tx_msg.header.DLC = 4;
	tx_msg.to = &hcan2;

	uint16_t position = APPS_Data.apps_position;
	tx_msg.data[0] = position & 0xFF;
	tx_msg.data[1] = position >> 8;


	uint16_t pressure = APPS_Data.brake_pressure;
	tx_msg.data[2] = pressure & 0xFF;
	tx_msg.data[3] = pressure >> 8;

	// Send over CAN2
	osMessageQueuePut(CANTX_QHandle, &tx_msg, 0, 5);
}

