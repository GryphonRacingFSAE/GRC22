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


void startCAN1TxTask(){
	CANMsg txMsg;
	CAN_TxHeaderTypeDef tempTxHeader;

	while(1){
		if (osMessageQueueGet(CAN1_QHandle, &txMsg, NULL, osWaitForever) == osOK){
			tempTxHeader.StdId = txMsg.StdId;
			tempTxHeader.ExtId = txMsg.ExtId;
			tempTxHeader.IDE = txMsg.IDE;
			tempTxHeader.DLC = txMsg.DLC;
			tempTxHeader.RTR = CAN_RTR_DATA;
			tempTxHeader.TransmitGlobalTime = DISABLE;

			HAL_CAN_AddTxMessage(&hcan1, &tempTxHeader, txMsg.aData, NULL);

			myprintf("%X ",txMsg.StdId);
			int i;
			for(i = 0;i<txMsg.DLC;i++){
				myprintf("%02X",txMsg.aData[i]);
			}
			myprintf("\n\r");
		}

	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	//set flag for new message waiting
	if(hcan == &hcan1){
		osThreadFlagsSet(&CANRxTaskHandle, CAN1_FLAG);
	}else{
		osThreadFlagsSet(&CANRxTaskHandle, CAN2_FLAG);
	}
}

void startCANRxTask(){
	CAN_RxHeaderTypeDef tempRxHeader;
	uint8_t Msg_Data[8];
	uint32_t flagsSet = 0;
	while(1){

		//wait for a message to be recieved on a CAN interface
		flagsSet = osThreadFlagsWait(CAN1_FLAG | CAN2_FLAG, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

		if(flagsSet & CAN1_FLAG){
			//clear flag
			osThreadFlagsClear(CAN1_FLAG);

			//get message from fifo and send to msg handler
			HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &tempRxHeader, Msg_Data);
			MsgHandler(&tempRxHeader, Msg_Data);

			//set flag if more messages remain in fifo
			if(HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0){
				osThreadFlagsSet(CANRxTaskHandle, CAN1_FLAG);
			}

		} else if(flagsSet & CAN2_FLAG){
			osThreadFlagsClear(CAN2_FLAG);

			HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &tempRxHeader, Msg_Data);
			MsgHandler(&tempRxHeader, Msg_Data);

			if(HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0){
				osThreadFlagsSet(CANRxTaskHandle, CAN2_FLAG);
			}
		}
	}
}


void MsgHandler(CAN_RxHeaderTypeDef *msgHeader, uint8_t msgData[]){
	//TODO handle CAN messages
}
