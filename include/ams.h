
#ifndef AMS_H
#define AMS_H

#define AMS_TASK_PERIOD (10)

void amsRisingEdgeInterrupt();
void startAMSTask(void* pvParameters);

#endif // AMS_H