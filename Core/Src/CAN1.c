/*
 * CAN1.c
 *
 *  Created on: Jan 16, 2023
 *      Author: Matt
 */
#include "CAN1.h"
#include "utils.h"
#include "control.h"
#include "APPS.h"
#include <string.h>

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

// INFO: Because we only have one task for receiving messages from CAN, all CAN inputs can be considered "serial"
void startCANRxTask() {
	CAN_RxHeaderTypeDef rxHeader;
	uint8_t canData[8];
	uint32_t flagsSet = 0;

	while (1) {
		// Wait for a message to be received on a CAN interface
		flagsSet = osThreadFlagsWait(CAN1_FLAG | CAN2_FLAG,
		osFlagsWaitAny | osFlagsNoClear, osWaitForever);

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
	switch (msgHeader->StdId) {
	case 0x0A2: { //INV_Hot_Spot_Temp, INV_Coolant_Temp
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_Hot_Spot_Temp = ((uint16_t) msgData[3] << 8) | ((uint16_t) msgData[2]);
			Ctrl_Data.motorControllerTemp = *(int16_t*) (&INV_Hot_Spot_Temp);
			uint16_t INV_Coolant_Temp = ((uint16_t) msgData[1] << 8) | ((uint16_t) msgData[0]);
			Ctrl_Data.coolantTemp = *(int16_t*) (&INV_Coolant_Temp);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	case 0x0A7: { // INV_DC_Bus_Voltage
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_DC_Bus_Voltage = ((uint16_t) msgData[1] << 8) | ((uint16_t) msgData[0]);
			Ctrl_Data.tractiveVoltage = *(int16_t*) (&INV_DC_Bus_Voltage);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	case 0x0A5: { // INV_Motor_Speed
		if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
			uint16_t INV_Motor_Speed = ((uint16_t) msgData[3] << 8) | ((uint16_t) msgData[2]);
			Ctrl_Data.motorSpeed = *(int16_t*) (&INV_Motor_Speed);
			osMutexRelease(Ctrl_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): CAN.c:canMsgHandler\n");
		}
		break;
	}
	case 0x0D0: // Transaction initiation
		initiateTransaction(msgHeader, msgData);
		break;
	case 0x0D1: // Transaction packet
		handleTransactionPacket(msgHeader, msgData);
		break;
	}
}

Transaction_Data_Struct Transaction_Data = { {}, '\0', 0, 0, CAN_TRANSACTION_PAUSED, {} };

void sendTransactionResponse(Transaction_Response_Struct* response) {
    DEBUG_PRINT("Sending response for transaction with ID: %d", transactionHeader.id);
    CANMsg response_packet = {
        .header = {
            .DLC = sizeof(response),
            .StdId = 0x0D2
        }
    };
    
    // Transaction response only contains uint8_t as effectively uint8_t[8]. So this should be a safe operation
    memcpy(&response_packet.aData, &response, sizeof(response));

    osMessageQueuePut(CAN1_QHandle, &response_packet, 0, 5); // TODO: should send from whichever interface it was received on
}

void initiateTransaction(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]) {
	if (msgHeader->DLC != sizeof(Transaction_Header_Struct)) { // Transactions have a required length of 8
        // TODO: determine if we should even send a nak here...
        // ???: If the CAN message isn't the correct size, can it even be considered a transmission request?
		WARNING_PRINT("Transaction DLC is invalid: %d\n", msgHeader->DLC);
		return;
	}

    Transaction_Header_Struct transactionHeader;
    // Transaction Header only contains uint8_t as effectively uint8_t[8]. So this should be a safe operation
    memcpy(&transactionHeader, msgData, sizeof(transactionHeader));

    // Verify max size of transaction
    if (transactionHeader.size > 255) {
        WARNING_PRINT("Requested transaction size too large, sending nak for: %d\n", transactionHeader.id);
        Transaction_Response_Struct nak_info = {
            .id = transactionHeader.id,
            .flags = CAN_TRANSACTION_HEADER_INVALID | CAN_TRANSACTION_NAK
        }
        sendTransactionResponse(&nak_info);
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
                }
                sendTransactionResponse(&nak_info);
                break;
            }
            break;
        }
        default: {
            ERROR_PRINT("Unknown transaction type, sending nak for: %d\n", transactionHeader.id);
            Transaction_Response_Struct nak_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_UNKNOWN_TYPE | CAN_TRANSACTION_HEADER_INVALID | CAN_TRANSACTION_NAK
            }
            sendTransactionResponse(&nak_info);
            break;
        }
    }

    // IDEA: As this function is "serial" and won't be called in parallel, is there even a point in needing a mutex? 
	if (osMutexAcquire(Transaction_Data_MtxHandle, osWaitForever) == osOK) {
		if (Transaction_Data.flags & CAN_TRANSACTION_PAUSED) {
            Transaction_Data.currentTransactionSize = 0;
            Transaction_Data.transactionInfo = transactionHeader;
            Transaction_Data.flags &= ~CAN_TRANSACTION_PAUSED; // Unpause transaction

            DEBUG_PRINT("Sending ack for transaction with ID: %d", transactionHeader.id);
            Transaction_Response_Struct ack_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_ACK
            }
            sendTransactionResponse(&ack_info);
		} else {
			WARNING_PRINT("Received transaction initiation, but transaction is already in progress, sending nak for: %d\n", transactionHeader.id);
            Transaction_Response_Struct nak_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_BUSY | CAN_TRANSACTION_NAK
            }
            sendTransactionResponse(&nak_info);
		}
		osMutexRelease(Transaction_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(Transaction_Data_MtxHandle): CAN.c:initiateTransaction\n");
        WARNING_PRINT("Received transaction initiation, but couldn't modify internal buffer, sending nak for: %d\n", transactionHeader.id);
        Transaction_Response_Struct nak_info = {
            .id = transactionHeader.id,
            .flags = CAN_TRANSACTION_INTERNAL_ERROR | CAN_TRANSACTION_NAK
        }
        sendTransactionResponse(&nak_info);
	}
}

// FIXME: buffer underrun will leave our transaction handler in an invalid state, rejecting new transactions until it completes or is filled with invalid data.
// IDEA: add timeout for packets, I.E. 500ms maximum
void handleTransactionPacket(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]) {
    // IDEA: As this function is "serial" and won't be called in parallel, is there even a point in needing a mutex? 
	if (osMutexAcquire(Transaction_Data_MtxHandle, osWaitForever) == osOK) {
		if (Transaction_Data.flags & CAN_TRANSACTION_PAUSED) {
			WARNING_PRINT("Received transaction packet, but transaction is paused, ignoring\n");
            Transaction_Response_Struct nak_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_INACTIVE | CAN_TRANSACTION_MESSAGE_INVALID | CAN_TRANSACTION_NAK
            }
            sendTransactionResponse(&nak_info);
		} else if ( // Check if too much data is trying to be sent
            Transaction_Data.currentSize + msgHeader->DLC > 255 ||
            Transaction_Data.currentSize + msgHeader->DLC > Transaction_Data.header.size
        ) { 
            ERROR_PRINT("Attempted buffer overrun: CAN.c:handleTransactionPacket\n");
            Transaction_Response_Struct nak_info = {
                .id = transactionHeader.id,
                .flags = CAN_TRANSACTION_BUFFER_OVERRUN | CAN_TRANSACTION_MESSAGE_INVALID | CAN_TRANSACTION_NAK
            }
            sendTransactionResponse(&nak_info);
            Transaction_Data.flags |= CAN_TRANSACTION_PAUSED; // Pause transaction
        } else {
            for (uint32_t count = 0; count < msgHeader->DLC; count++) {
                Transaction_Data.buffer[Transaction_Data.currentSize + count] = msgData[count];
            }
            Transaction_Data.currentSize += msgHeader->DLC;

            // Final Packet Handling
            if (Transaction_Data.currentSize == Transaction_Data.header.size) {
                switch (Transaction_Data.type) {
                case 'T': {

                    // This is the only spot where torque_maps are written to, we only need to protect the torque map that
                    // is being currently read from, if we are not writing to that or the selector, we don't need a mutex
                    // We need to convert our uint8_t buffer - uint8_t offset to our int16_t torque map values.
                    uint8_t offset = Transaction_Data.header.params[2];
                    if (Torque_Map_Data.activeMap != &Torque_Map_Data.map1) {
                        // Edit map2
                        uint32_t index = 0;
                        for (uint8_t row = 0; row < rowCount; row++){
                            for (uint8_t column = 0; column < columnCount; column++, index++){
                                Torque_Map_Data.map2.data[row][column] = (int16_t)Transaction_Data.buffer[index] - offset;
                            }
                        }
                    } else {
                        // Edit map1
                        uint32_t index = 0;
                        for (uint8_t row = 0; row < rowCount; row++){
                            for (uint8_t column = 0; column < columnCount; column++, index++){
                                Torque_Map_Data.map1.data[row][column] = (int16_t)Transaction_Data.buffer[index] - offset;
                            }
                        }
                    }

                    // As we are editing something that IS being read from, we now need the mutex
                    if (osMutexAcquire(Torque_Map_MtxHandle, osWaitForever) == osOK) {
                        // Swap which map is being used
                        if (Torque_Map_Data.activeMap != &Torque_Map_Data.map1) {
                            Torque_Map_Data.activeMap = &Torque_Map_Data.map2;
                        } else {
                            Torque_Map_Data.activeMap = &Torque_Map_Data.map1;
                        }
                        osMutexRelease(Torque_Map_MtxHandle);
                    }
                    DEBUG_PRINT("Sending ack for torque map with ID: %d", transactionHeader.id);
                    Transaction_Response_Struct ack_info = {
                        .id = transactionHeader.id,
                        .flags = CAN_TRANSACTION_ACK
                    }
                    sendTransactionResponse(&ack_info);
                    break;
                }
                default:
                    // TODO: this is handled when the initial transaction request comes in
                    CRITICAL_PRINT("Unknown transaction type in handleTransmissionPacket, should never be reached\n");
                    break;
                }
            }
		}
		osMutexRelease(Transaction_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(Transaction_Data_MtxHandle): CAN.c:handleTransmissionPacket\n");
	}
}
