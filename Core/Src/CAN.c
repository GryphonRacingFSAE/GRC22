#include "CAN.h"
#include "utils.h"
#include "main.h"

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

// INFO: Because we only have one task for receiving messages from CAN, all CAN inputs can be considered "serial"
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
}
