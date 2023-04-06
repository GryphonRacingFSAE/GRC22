#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"

static void twai_transmit_task()
{
    twai_message_t tx_msg = {
        .data_length_code = 4,
        .identifier = 0x123,
        .self = 1};

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < tx_msg.data_length_code; j++)
        {
            tx_msg.data[j] = i;
        }
        ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

static void twai_receive_task()
{
    twai_message_t rx_msg;

    for (int i = 0; i < 100; i++)
    {
        ESP_ERROR_CHECK(twai_receive(&rx_msg, portMAX_DELAY));
        printf("ID: 0x%x  Data:", rx_msg.identifier);
        for (int i = 0; i < rx_msg.data_length_code; i++)
        {
            printf(" %02x", rx_msg.data[i]);
        }
        printf("\n");
    }
    vTaskDelete(NULL);
}

void app_main()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NO_ACK);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    xTaskCreate(twai_transmit_task, "twai_transmit_task", 2048, NULL, 5, NULL);
    xTaskCreate(twai_receive_task, "twai_receive_task", 2048, NULL, 5, NULL);
}