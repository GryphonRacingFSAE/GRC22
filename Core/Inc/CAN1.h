/*
 * CAN1.h
 *
 *  Created on: Jan 16, 2023
 *      Author: Matt
 */

#ifndef INC_CAN1_H_
#define INC_CAN1_H_

#include "main.h"
#include "cmsis_os.h"

extern osMessageQueueId_t CAN1_QHandle;

#define CAN1_FLAG 0x00000001U
#define CAN2_FLAG 0x00000002U

typedef struct {
	CAN_TxHeaderTypeDef header;

	uint8_t aData[8];
} CANMsg;


extern osMutexId_t Transaction_Data_MtxHandle;

#define CAN_TRANSACTION_PAUSED 0x01

typedef struct {
	uint8_t buffer[256];
	uint8_t type;
	uint8_t transactionSize;
	uint8_t currentTransactionSize;
	uint8_t flags;
	uint8_t transactionInfo[4];
} Transaction_Data_Struct;

extern Transaction_Data_Struct Transaction_Data;


void startCAN1TxTask();
void startCANRxTask();
void canMsgHandler(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]);
void initiateTransaction(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]);
void handleTransactionPacket(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]);

#endif /* INC_CAN1_H_ */
