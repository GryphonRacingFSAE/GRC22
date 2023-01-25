/*
 * CAN1.c
 *
 *  Created on: Jan 16, 2023
 *      Author: Matt
 */
#include "CAN1.h"
#include "utils.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern osThreadId_t CANRxTaskHandle;

void startCAN1TxTask() {
	CANMsg txMsg;

	while (1) {
		// Grab CAN message from CAN1 queue
		if (osMessageQueueGet(CAN1_QHandle, &txMsg, NULL, osWaitForever) == osOK) {

			// Send out TX message on CAN
			HAL_CAN_AddTxMessage(&hcan1, &(txMsg.header), txMsg.aData, NULL);

			// Print out the TX message
			myprintf("%X ", txMsg.header.StdId);
			for (int i = 0; i < txMsg.header.DLC; i++) {
				myprintf("%02X", txMsg.aData[i]);
			}
			myprintf("\n");
		}

	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	// Set flag to determine where message was received from
	if (hcan == &hcan1) {
		osThreadFlagsSet(&CANRxTaskHandle, CAN1_FLAG);
	} else {
		osThreadFlagsSet(&CANRxTaskHandle, CAN2_FLAG);
	}
}

void startCANRxTask() {
	CAN_RxHeaderTypeDef rxHeader;
	uint8_t canData[8];
	uint32_t flagsSet = 0;

	while (1) {
		// Wait for a message to be received on a CAN interface
		flagsSet = osThreadFlagsWait(CAN1_FLAG | CAN2_FLAG, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

		if (flagsSet & CAN1_FLAG) {
			osThreadFlagsClear(CAN1_FLAG);

			// Receive message from CAN1's FIFO buffer, and forward to the message handler
			HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, canData);
			canMsgHandler(&rxHeader, canData);

			// If elements still exist in the buffer, set the flag again
			if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
				osThreadFlagsSet(CANRxTaskHandle, CAN1_FLAG);
			}

		} else if (flagsSet & CAN2_FLAG) {
			osThreadFlagsClear(CAN2_FLAG);

			// Receive message from CAN2's FIFO buffer, and forward to the message handler
			HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &rxHeader, canData);
			canMsgHandler(&rxHeader, canData);

			// If elements still exist in the buffer, set the flag again
			if (HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0) {
				osThreadFlagsSet(CANRxTaskHandle, CAN2_FLAG);
			}
		}
	}
}

void canMsgHandler(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]) {
	//TODO handle CAN messages
}
