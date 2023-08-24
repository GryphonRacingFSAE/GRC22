#include "CAN.h"
#include "utils.h"
#include "main.h"

void startCANTxTask() {
	CANTXMsg txMsg;

	uint32_t mailbox = 0;
	while (1) {
		// Grab CAN message from CAN1 queue
		if (osMessageQueueGet(CANTX_QHandle, &txMsg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			TRACE_PRINT("CAN1 sending message: %d or %d\r\n", txMsg.header.StdId, txMsg.header.ExtId);
			if (HAL_CAN_AddTxMessage(txMsg.to, &txMsg.header, txMsg.data, &mailbox) != HAL_OK) {
				ERROR_PRINT("Could not transmit on CAN!\r\n");
			}
		}
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CANRXMsg rx_msg = { .from = hcan }; // Allow the message handler to know where to send any responses to.
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_msg.header, rx_msg.data);
	TRACE_PRINT("CAN received message: %d to address: %ld\r\n", (uint32_t)(hcan), (uint32_t)rx_msg.header.ExtId);
	// We don't actually need to receive any data on this device, so printing and moving on is totally fine.
}
