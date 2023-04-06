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

void startCAN1TxTask() {
	CANTXMsg txMsg;

	while (1) {
		// Grab CAN message from CAN1 queue
		if (osMessageQueueGet(CAN1_QHandle, &txMsg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			uint32_t mailbox_location = 0;
			TRACE_PRINT("CAN1 sending message: %d or %d\r\n", txMsg.header.StdId, txMsg.header.ExtId);
			if (HAL_CAN_AddTxMessage(&hcan1, &txMsg.header, txMsg.data, &mailbox_location) != HAL_OK) {
				ERROR_PRINT("Could not transmit on CAN1!\r\n");
			}
		}
	}
}

void startCAN2TxTask() {
	CANTXMsg txMsg;

	while (1) {
		// Grab CAN message from CAN2 queue
		if (osMessageQueueGet(CAN2_QHandle, &txMsg, NULL, osWaitForever) == osOK) {
			// Send out TX message on CAN
			TRACE_PRINT("CAN2 sending message: %d or %d\r\n", txMsg.header.StdId, txMsg.header.ExtId);
			uint32_t mailbox_location = 0;
			if (HAL_CAN_AddTxMessage(&hcan2, &txMsg.header, txMsg.data, &mailbox_location) != HAL_OK) {
				ERROR_PRINT("Could not transmit on CAN2!\r\n");
			}
		}
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CANRXMsg rxMsg = { .from = hcan }; // Allow the message handler to know where to send any responses to.
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxMsg.header, rxMsg.data);
	osMessageQueuePut(CANRX_QHandle, &rxMsg, 0, 0);
	DEBUG_PRINT("CAN received message: %d or %d\r\n", rxMsg.header.StdId, rxMsg.header.ExtId);
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
	case 0x0D0: // Transaction initiation
		initiateTransaction(rxMsg);
		break;
	case 0x0D1: // Transaction packet
		handleTransactionPacket(rxMsg);
		break;
	}
}

Transaction_Data_Struct Transaction_Data = { {}, CAN_TRANSACTION_PAUSED, 0, { 0, '\0', 0 }};

void sendTransactionResponse(Transaction_Response_Struct* response, CAN_HandleTypeDef* target_bus) {
    DEBUG_PRINT("Sending response for transaction with ID: %d", response->id);
    CANTXMsg response_packet = {
        .header = {
            .DLC = sizeof(response),
            .StdId = 0x0D2
        }
    };
    
    // Transaction response only contains uint8_t as effectively uint8_t[8]. So this should be a safe operation
    memcpy(&response_packet.data, &response, sizeof(response));

    if (target_bus == &hcan1) {
    	osMessageQueuePut(CAN1_QHandle, &response_packet, 0, 5);
    } else if (target_bus == &hcan2) {
    	osMessageQueuePut(CAN2_QHandle, &response_packet, 0, 5);
    } else {
    	ERROR_PRINT("Unknown target bus value: %p", target_bus);
    }
}

void initiateTransaction(CANRXMsg* rxMsg) {
	if (rxMsg->header.DLC != sizeof(Transaction_Header_Struct)) { // Transactions have a required length of 8
        // TODO: determine if we should even send a nak here...
        // ???: If the CAN message isn't the correct size, can it even be considered a transmission request?
		WARNING_PRINT("Transaction DLC is invalid: %d\n", rxMsg->header.DLC);
		return;
	}

    Transaction_Header_Struct transactionHeader;
    // Transaction Header only contains uint8_t as effectively uint8_t[8]. So this should be a safe operation
    memcpy(&transactionHeader, rxMsg->data, sizeof(transactionHeader));

    // Transaction can't have size 0
    if (transactionHeader.size != 0) {
        WARNING_PRINT("Transaction must have non-zero size, sending nak for: %d\n", transactionHeader.id);
        Transaction_Response_Struct nak_info = {
            .id = transactionHeader.id,
            .flags = CAN_TRANSACTION_HEADER_INVALID | CAN_TRANSACTION_NAK
        };
        sendTransactionResponse(&nak_info, rxMsg->from);
        return;
    }

    switch (transactionHeader.type) {
        case 'T':  {
            // Force the torque map to a specified size currently
            uint8_t columnCount = transactionHeader.params[0];
            uint8_t rowCount = transactionHeader.params[1];
            if (columnCount != TORQUE_MAP_COLUMNS || rowCount != TORQUE_MAP_ROWS) {
                WARNING_PRINT("Torque map transaction has incorrect dimensions: (%dx%d), sending nak for %d\n", columnCount, rowCount, transactionHeader.id);
                Transaction_Response_Struct nak_info = {
                    .id = transactionHeader.id,
                    .flags = CAN_TRANSACTION_INVALID_PARAMS | CAN_TRANSACTION_HEADER_INVALID | CAN_TRANSACTION_NAK
                };
                sendTransactionResponse(&nak_info, rxMsg->from);
                return;
            }
            break;
        }
        default: {
            ERROR_PRINT("Unknown transaction type, sending nak for: %d\n", transactionHeader.id);
            Transaction_Response_Struct nak_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_UNKNOWN_TYPE | CAN_TRANSACTION_HEADER_INVALID | CAN_TRANSACTION_NAK
            };
            sendTransactionResponse(&nak_info, rxMsg->from);
            return;
        }
    }

    if (!(Transaction_Data.flags & CAN_TRANSACTION_PAUSED)) {
        WARNING_PRINT("Received transaction initiation, but transaction is already in progress, sending nak for: %d\n", transactionHeader.id);
        Transaction_Response_Struct nak_info = {
            .id = transactionHeader.id,
            .flags = CAN_TRANSACTION_BUSY | CAN_TRANSACTION_NAK
        };
        sendTransactionResponse(&nak_info, rxMsg->from);
        return;
    }

    // All checks were successful
    Transaction_Data.currentSize = 0;
    Transaction_Data.header = transactionHeader;
    Transaction_Data.flags &= ~CAN_TRANSACTION_PAUSED; // Unpause transaction

    DEBUG_PRINT("Sending ack for transaction with ID: %d", transactionHeader.id);
    Transaction_Response_Struct ack_info = {
        .id = transactionHeader.id,
        .flags = CAN_TRANSACTION_ACK
    };
    sendTransactionResponse(&ack_info, rxMsg->from);
}

// FIXME: buffer underrun will leave our transaction handler in an invalid state, rejecting new transactions until it completes or is filled with invalid data.
// IDEA: add timeout for packets, I.E. 500ms maximum
inline void handleTransactionPacket(CANRXMsg* rxMsg) {
    if (Transaction_Data.flags & CAN_TRANSACTION_PAUSED) {
        WARNING_PRINT("Received transaction packet, but transaction is paused, ignoring\n");
        Transaction_Response_Struct nak_info = {
            .id = Transaction_Data.header.id,
            .flags = CAN_TRANSACTION_INACTIVE | CAN_TRANSACTION_MESSAGE_INVALID | CAN_TRANSACTION_NAK
        };
        sendTransactionResponse(&nak_info, rxMsg->from);
        return;
    }
    
    if ( // Check if too much data is trying to be sent
        Transaction_Data.currentSize + rxMsg->header.DLC > 255 ||
        Transaction_Data.currentSize + rxMsg->header.DLC > Transaction_Data.header.size
    ) { 
        ERROR_PRINT("Attempted buffer overrun: CAN.c:handleTransactionPacket\n");
        Transaction_Response_Struct nak_info = {
            .id = Transaction_Data.header.id,
            .flags = CAN_TRANSACTION_BUFFER_OVERRUN | CAN_TRANSACTION_MESSAGE_INVALID | CAN_TRANSACTION_NAK
        };
        sendTransactionResponse(&nak_info, rxMsg->from);
        Transaction_Data.flags |= CAN_TRANSACTION_PAUSED; // Pause transaction
        return;
    }

    // Insert packet into buffer
    memcpy(Transaction_Data.buffer + Transaction_Data.currentSize, rxMsg->data, rxMsg->header.DLC);
    Transaction_Data.currentSize += rxMsg->header.DLC;

    // Final Packet Handling
    if (Transaction_Data.currentSize == Transaction_Data.header.size) {
        switch (Transaction_Data.header.type) {
        case 'T': {
            uint8_t columnCount = Transaction_Data.header.params[0];
            uint8_t rowCount = Transaction_Data.header.params[1];
            uint8_t offset = Transaction_Data.header.params[2];

            // This is the only spot where torque_maps are written to, we only need to protect the torque map that
            // is being currently read from, if we are not writing to that or the selector, we don't need a mutex
            // We need to convert our uint8_t buffer - uint8_t offset to our int16_t torque map values.
            if (Torque_Map_Data.activeMap == Torque_Map_Data.map1) {
                // Edit map2
                uint32_t index = 0;
                for (uint8_t row = 0; row < rowCount; row++){
                    for (uint8_t column = 0; column < columnCount; column++, index++){
                        Torque_Map_Data.map2[row][column] = (int16_t)Transaction_Data.buffer[index] - offset;
                    }
                }
            } else {
                // Edit map1
                uint32_t index = 0;
                for (uint8_t row = 0; row < rowCount; row++){
                    for (uint8_t column = 0; column < columnCount; column++, index++){
                        Torque_Map_Data.map1[row][column] = (int16_t)Transaction_Data.buffer[index] - offset;
                    }
                }
            }

            // As we are editing something that IS being read from, we now need the mutex
            if (osMutexAcquire(Torque_Map_MtxHandle, osWaitForever) == osOK) {
                // Swap which map is being used
                if (Torque_Map_Data.activeMap == Torque_Map_Data.map1) {
                    Torque_Map_Data.activeMap = Torque_Map_Data.map2;
                } else {
                    Torque_Map_Data.activeMap = Torque_Map_Data.map1;
                }
                osMutexRelease(Torque_Map_MtxHandle);
            } else {
                ERROR_PRINT("Could not modify torque map, sending nak for: %d\n", Transaction_Data.header.id);
                Transaction_Response_Struct nak_info = {
                    .id = Transaction_Data.header.id,
                    .flags = CAN_TRANSACTION_INTERNAL_ERROR | CAN_TRANSACTION_NAK
                };
                sendTransactionResponse(&nak_info, rxMsg->from);
                return;
            }

            DEBUG_PRINT("Sending ack for torque map with ID: %d", Transaction_Data.header.id);
            Transaction_Response_Struct ack_info = {
                .id = Transaction_Data.header.id,
                .flags = CAN_TRANSACTION_ACK
            };
            sendTransactionResponse(&ack_info, rxMsg->from);
            break;
        }
        default:
            // INFO: This is handled when the initial transaction request comes in
            CRITICAL_PRINT("Unknown transaction type in handleTransmissionPacket, should never be reached\n");
            break;
        }
    }
}
