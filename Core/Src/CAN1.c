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

void initiateTransaction(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]) {
	if (msgHeader->DLC != 6) { // Transactions have a minimum length of 2
		WARNING_PRINT("Transaction DLC is invalid: %d\n", msgHeader->DLC);
		return;
	}

	if (osMutexAcquire(Transaction_Data_MtxHandle, osWaitForever) == osOK) {
		if (Transaction_Data.flags & CAN_TRANSACTION_PAUSED) {
			Transaction_Data.type = msgData[0];
			Transaction_Data.transactionSize = msgData[1];
			Transaction_Data.currentTransactionSize = 0;
			Transaction_Data.transactionInfo[0] = msgData[2];
			Transaction_Data.transactionInfo[1] = msgData[3];
			Transaction_Data.transactionInfo[2] = msgData[4];
			Transaction_Data.transactionInfo[3] = msgData[5];
			Transaction_Data.flags &= ~CAN_TRANSACTION_PAUSED; // Unpause transaction
		} else {
			ERROR_PRINT("Received transaction initiation, but transaction is already in progress, ignoring transactions - ERRORS WILL PROCCED\n");
		}
		osMutexRelease(Transaction_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(Transaction_Data_MtxHandle): CAN.c:initiateTransaction\n");
	}
}

void handleTransactionPacket(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]) {
	if (osMutexAcquire(Transaction_Data_MtxHandle, osWaitForever) == osOK) {
		if (Transaction_Data.flags & CAN_TRANSACTION_PAUSED) {
			WARNING_PRINT("Received transaction packet, but transaction is paused, ignoring\n");
		} else {
			// Buffer overrun checks
			if (Transaction_Data.currentTransactionSize + msgHeader->DLC > 255) {
				// Buffer overrun
				ERROR_PRINT("Attempted buffer overrun: CAN.c:handleTransactionPacket\n");
				Transaction_Data.flags |= CAN_TRANSACTION_PAUSED; // Pause transaction
			} else if (Transaction_Data.currentTransactionSize + msgHeader->DLC > Transaction_Data.transactionSize) {
				// Too much data
				ERROR_PRINT("Too much data received for transaction: CAN.c:handleTransactionPacket\n");
				Transaction_Data.flags |= CAN_TRANSACTION_PAUSED; // Pause transaction
			} else {
				for (uint32_t count = 0; count < msgHeader->DLC; count++) {
					Transaction_Data.buffer[Transaction_Data.currentTransactionSize + count] = msgData[count];
				}
				Transaction_Data.currentTransactionSize += msgHeader->DLC;

				if (Transaction_Data.currentTransactionSize == Transaction_Data.transactionSize) {
					switch (Transaction_Data.type) {
					case 'T': {
						uint8_t columnCount = Transaction_Data.transactionInfo[0];
						uint8_t rowCount = Transaction_Data.transactionInfo[1];

						if (columnCount != TORQUE_MAP_COLUMNS || rowCount != TORQUE_MAP_ROWS) {
							WARNING_PRINT("Torque map transaction has incorrect dimensions : (%dx%d)\n", columnCount, rowCount);
							break;
						}

						int8_t offset = *(int8_t*) &Transaction_Data.transactionInfo[2];

						// This is the only spot where torque_maps are written to, we only need to protect the torque map that
						// is being currently read from, if we are not writing to that or the selector, we don't need a mutex
						if (Torque_Map_Data.activeMap != &Torque_Map_Data.map1) {
							// Edit map2
							Torque_Map_Data.map2.offset = offset;
							memcpy(Torque_Map_Data.map2.data, Transaction_Data.buffer, Transaction_Data.transactionSize);
						} else {
							// Edit map1
							Torque_Map_Data.map1.offset = offset;
							memcpy(Torque_Map_Data.map1.data, Transaction_Data.buffer, Transaction_Data.transactionSize);
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
						break;
					}
					default:
						WARNING_PRINT("Unknown transaction type\n");
						break;
					}
				}
			}
		}
		osMutexRelease(Transaction_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(Transaction_Data_MtxHandle): CAN.c:initiateTransaction\n");
	}
}
