/*
 * CAN1.c
 *
 *  Created on: Jan 16, 2023
 *      Author: Matt, David, Dallas Hart
 */
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

		// Send messages that should be transmitted every 1 second
		if (tick % 1000 == 0) {

		}

		osDelayUntil(tick += CAN_TRANSMIT_PERIOD);
	}
}

void sendTorque() {
	DEBUG_PRINT("Requesting: %dN.m\r\n", requestedTorque);
	requestedTorque *= 10; // Scaling is 10:1, requested torque is what is requested, it needs to be sent as 10 this value

	// Format is defined in CM200DZ CAN protocol V6.1 section 2.2
	CANTXMsg tx_message;
	tx_msg.header.IDE = CAN_ID_STD;
	tx_msg.header.RTR = CAN_RTR_DATA;
	tx_message.header.StdId = 0x0C0;
	tx_message.header.DLC = 8;
	tx_message.to = &hcan2;


	// Bytes 0 & 1 is the requested torque
	uint16_t bitwise_requested_torque = *(uint16_t*)&requestedTorque;
	tx_msg.data[0] = bitwiseRequestedTorque & 0xFF;
	tx_msg.data[1] = bitwiseRequestedTorque >> 8;

	// Bytes 2 & 3 is the requested RPM (if not in torque mode)
	tx_msg.data[2] = 0;
	tx_message.data[3] = 0;

	// Byte 4 is Forward/Reverse
	tx_msg.data[4] = 1; // 1 is Forward

	// Byte 5 is Configuration
	tx_msg.data[5] = 0;
		// | 0x1 // Inverter Enable
		// | 0x2 // Inverter Discharge
		// | 0x4 // Speed Mode override

	// Byte 6 & 7 sets torque limits
	tx_message.data[6] = 0;
	tx_message.data[7] = 0;

	// Send over CAN2
	osMessageQueuePut(CANTX_QHandle, &tx_msg, 0, 5);
}


void startCANTxTask() {
	CANTXMsg txMsg;

	while (1) {
		// Grab CAN message from CAN1 queue
		if (osMessageQueueGet(CANTX_QHandle, &txMsg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			TRACE_PRINT("CAN1 sending message: %d or %d\r\n", txMsg.header.StdId, txMsg.header.ExtId);
			if (HAL_CAN_AddTxMessage(txMsg.to, &txMsg.header, txMsg.data, NULL) != HAL_OK) {
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

// INFO: Because we only have one tasxk for receiving messages from CAN, all CAN inputs can be considered "serial"
void startCANRxTask() {
	CANRXMsg rxMsg;

	while (1) {
		// Grab CAN message from RX queue
		if (osMessageQueueGet(CANRX_QHandle, &rxMsg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			DEBUG_PRINT("CAN receiving message: %d or %d\r\n", rxMsg.header.StdId, rxMsg.header.ExtId);
			canMsgHandler(&rxMsg);
		}
	}
}

inline void canMsgHandler(CANRXMsg* rxMsg) {
	DEBUG_PRINT("CAN receiving message: %d or %d\n", rxMsg->header.StdId, rxMsg->header.ExtId);
	switch (rxMsg->header.StdId) {
	case 0x0A2: { //INV_Hot_Spot_Temp, INV_Coolant_Temp
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_Hot_Spot_Temp = ((uint16_t) rxMsg->data[3] << 8) | ((uint16_t) rxMsg->data[2]);
			Ctrl_Data.motorControllerTemp = *(int16_t*) (&INV_Hot_Spot_Temp);
			uint16_t INV_Coolant_Temp = ((uint16_t) rxMsg->data[1] << 8) | ((uint16_t) rxMsg->data[0]);
			Ctrl_Data.coolantTemp = *(int16_t*) (&INV_Coolant_Temp);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	case 0x0A7: { // INV_DC_Bus_Voltage
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_DC_Bus_Voltage = ((uint16_t) rxMsg->data[1] << 8) | ((uint16_t) rxMsg->data[0]);
			Ctrl_Data.tractiveVoltage = *(int16_t*) (&INV_DC_Bus_Voltage);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	case 0x0A5: { // INV_Motor_Speed
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_Motor_Speed = ((uint16_t) rxMsg->data[3] << 8) | ((uint16_t) rxMsg->data[2]);
			Ctrl_Data.motorSpeed = *(int16_t*) (&INV_Motor_Speed);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	}
}
