/*
 * CAN1.h
 *
 *  Created on: Jan 16, 2023
 *      Author: Matt
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "main.h"
#include "cmsis_os.h"

extern osMessageQueueId_t CANTX_QHandle;
extern osMessageQueueId_t CANRX_QHandle;

typedef struct {
	CAN_TxHeaderTypeDef header;
	CAN_HandleTypeDef* to; // CAN port that message should be sent to
	uint8_t data[8];
} CANTXMsg;

typedef struct {
	CAN_RxHeaderTypeDef header;
	CAN_HandleTypeDef* from; // CAN port that message was received from
	uint8_t data[8];
} CANRXMsg;

void startCAN1TxTask();
void startCAN2TxTask();
void startCANRxTask();
void canMsgHandler(CANRXMsg* rxMsg);

#endif /* INC_CAN_H_ */
