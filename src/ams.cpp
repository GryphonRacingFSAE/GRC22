#include "ams.h"
#include "globals.h"
#include "utils.h"

#include <driver/twai.h>
#include <freertos/task.h>

static uint16_t ams_chatter_count = 0;

void IRAM_ATTR amsRisingEdgeInterrupt() {
    ams_chatter_count++;
}

void startAMSTask(void* params) {
    (void)params;
    uint16_t loop_count = 0;
    TickType_t tick = xTaskGetTickCount();
    while (1) {
        if (loop_count * AMS_TASK_PERIOD >= 1000) {
            loop_count = 0;
            if (ams_chatter_count >= 4) {
                digitalWrite(AMS_SHUTDOWN_PIN, LOW);
                SET_FLAG(global_output_peripherals.flags, AMS_CHATTER_ACTIVE);
                xTaskDelayUntil(&tick, pdMS_TO_TICKS(AMS_TASK_PERIOD));
                ams_chatter_count = 0;
                continue;
            }
            ams_chatter_count = 0;
        }

        uint32_t can_error = 0;
        twai_read_alerts(&can_error, 0);
        // Serial.printf("DTC1: %04x, DTC2: %04x\n", global_bms.DTC1, global_bms.DTC2);
        if (!can_error && (tick < (global_bms.last_heartbeat + 1000))) {
            if ((global_bms.DTC1 & 0x00FF) || (global_bms.DTC2 & 0xFFF2)) {
                digitalWrite(AMS_SHUTDOWN_PIN, LOW);
            } else {
                digitalWrite(AMS_SHUTDOWN_PIN, HIGH);
            }
        } else {
            digitalWrite(AMS_SHUTDOWN_PIN, LOW);
        }

        xTaskDelayUntil(&tick, pdMS_TO_TICKS(AMS_TASK_PERIOD));
        loop_count += 1;
    }
}