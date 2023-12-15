#include "TEMInterface.h"
#include "ThermistorMonitor.h"
#include "main.h"
#include "utils.h"
#include <CAN.h>


extern CAN_HandleTypeDef hcan;

CANTXMsg bms_broadcast = {.header = {.IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .ExtId = 0x1839F380, .DLC = 8}, .to = &hcan};

CANTXMsg general_broadcast = {.header = {.IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .ExtId = 0x1838F380, .DLC = 8}, .to = &hcan};

void startTEMInterfaceTask() {
    uint32_t tick = osKernelGetTickCount();

    bms_broadcast.data[0] = 0x55;
    bms_broadcast.data[1] = 0x44;
    bms_broadcast.data[2] = 0x55;
    bms_broadcast.data[3] = 0x44;
    bms_broadcast.data[4] = 0x55;
    bms_broadcast.data[5] = 0x44;
    bms_broadcast.data[6] = 0x55;
    bms_broadcast.data[7] = 0x44;

    general_broadcast.data[0] = 0x55;
    general_broadcast.data[1] = 0x44;
    general_broadcast.data[2] = 0x55;
    general_broadcast.data[3] = 0x44;
    general_broadcast.data[4] = 0x55;
    general_broadcast.data[5] = 0x44;
    general_broadcast.data[6] = 0x55;
    general_broadcast.data[7] = 0x44;

    //	ERROR_PRINT("STARTING UP TEM INTERFACE TASK");

    while (1) {
        // GRCprintf("Sending out to queue");
        osMessageQueuePut(CANTX_QHandle, &bms_broadcast, 0, 0);
        osMessageQueuePut(CANTX_QHandle, &general_broadcast, 0, 0);
        osDelayUntil(tick += TEM_PERIOD);
    }
}
