#include <Arduino.h>
#include <HardwareSerial.h>
#include <freertos/task.h>

#include "ams.h"
#include "can.h"
#include "globals.h"
#include "peripherals.h"
#include "torque.h"
#include "utils.h"
#include <Preferences.h>

Preferences param_storage;

void setup() {
    Serial.begin(921600);

    initCAN();
    param_storage.begin("params", READ_WRITE_MODE);     

    ledcSetup(0, 50, 10); // 50Hz PWM, 10-bit resolution
    pinMode(PUMP_PWM_PIN, OUTPUT);
    ledcAttachPin(PUMP_PWM_PIN, 0); // assign RGB led pins to channels

    pinMode(APPS1_PIN, INPUT);
    pinMode(APPS2_PIN, INPUT);
    pinMode(BRAKE_PRESSURE_PIN, INPUT);
    pinMode(PUSH_BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BRAKE_LIGHT_PIN, OUTPUT);
    pinMode(AMS_SHUTDOWN_PIN, OUTPUT);
    pinMode(IMD_PWM_RISING_PIN, INPUT);
    pinMode(IMD_PWM_FALLING_PIN, INPUT);
    pinMode(AIR_CONTACT_PIN, INPUT);
    pinMode(FLOW_SENS1_PIN, INPUT);

    attachInterrupt(AIR_CONTACT_PIN, amsRisingEdgeInterrupt, RISING);
    attachInterrupt(FLOW_SENS1_PIN, flowSens1Frequency, RISING);
    attachInterrupt(IMD_PWM_RISING_PIN, imdRisingEdgeTime, RISING);
    attachInterrupt(IMD_PWM_FALLING_PIN, imdFallingEdgeTime, FALLING);

    xTaskCreate(startAMSTask, "AMS_TASK", 2048, NULL, 8, NULL);
    Serial.println("Finished creating task 0");
    xTaskCreate(startControlTask, "CONTROL_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 1");
    xTaskCreate(startPeripheralTask, "PERIPHERAL_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 2");
    xTaskCreate(startTorqueTask, "APPS_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 3");
    xTaskCreate(startReceiveCANTask, "CAN_RECEIVE_TASK", 8192, NULL, 3, NULL);
    Serial.println("Finished creating task 4");
    xTaskCreate(startTransmitCANTask, "CAN_TRANSMIT_TASK", 8192, NULL, 5, NULL);
    Serial.println("Finished creating task 5");
}

void loop() {}
