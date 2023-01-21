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
	uint32_t StdId; /*!< Specifies the standard identifier.
	 This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

	uint32_t ExtId; /*!< Specifies the extended identifier.
	 This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

	uint32_t IDE; /*!< Specifies the type of identifier for the message that will be transmitted.
	 This parameter can be a value of @ref CAN_identifier_type */

	uint32_t DLC; /*!< Specifies the length of the frame that will be transmitted.
	 This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

	uint8_t aData[8];
} CANMsg;

void startCAN1TxTask();
void startCANRxTask();
void canMsgHandler(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]);

#endif /* INC_CAN1_H_ */
