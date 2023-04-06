#include <stdio.h>
#include "driver/twai.h"

void app_main()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NO_ACK);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        printf("Driver installed\n");
    }
    else
    {
        printf("Failed to install driver\n");
        return;
    }

    if (twai_start() == ESP_OK)
    {
        printf("Driver started\n");
    }
    else
    {
        printf("Failed to start driver\n");
        return;
    }

    twai_message_t tx_msg;
    twai_message_t rx_msg;

    while (true)
    {
        tx_msg.identifier = 0x123;
        tx_msg.data_length_code = 4;
        tx_msg.data[0] = 0xAA;
        tx_msg.data[1] = 0xBB;
        tx_msg.data[2] = 0xCC;
        tx_msg.data[3] = 0xDD;

        if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1000)) == ESP_OK)
        {
            printf("Message queued for transmission\n");
        }
        else
        {
            printf("Failed to queue message for transmission\n");
        }

        if (twai_receive(&rx_msg, pdMS_TO_TICKS(10000)) == ESP_OK)
        {
            printf("Message received\n");

            printf("ID: 0x%x  Data:", rx_msg.identifier);
            if (!(rx_msg.rtr))
            {
                for (int i = 0; i < rx_msg.data_length_code; i++)
                {
                    printf(" %02X", rx_msg.data[i]);
                }
            }
            printf("\n");
        }
        else
        {
            printf("Failed to receive message\n");
            return;
        }
    }
}