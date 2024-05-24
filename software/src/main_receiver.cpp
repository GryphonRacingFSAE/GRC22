#include <Arduino.h>
#include <RF24.h>
#include <pb_decode.h>

#include "message.pb.h"

//==============================================================================
// Global
//==============================================================================

// GPIO
#define NRF_CE GPIO_NUM_4
#define NRF_CSN GPIO_NUM_5

// nRF24L01+
RF24 radio(NRF_CE, NRF_CSN);
const byte address[6] = "00001";
uint8_t nrf_buffer[128];

//==============================================================================
// nRF24L01+
//==============================================================================

void initNRF() {
    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }

    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.setAutoAck(false);
    radio.openReadingPipe(0, address);
    radio.startListening();
}

//==============================================================================
// Setup
//==============================================================================

void setup() {
    Serial.begin(921600);
    delay(500);

    Serial.println();

    initNRF();
}

//==============================================================================
// Loop
//==============================================================================

void loop() {
    if (radio.available()) {
        radio.read(&nrf_buffer, sizeof(nrf_buffer));

        ProtoMessage msg = ProtoMessage_init_default;

        pb_istream_t input_stream = pb_istream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
        pb_decode(&input_stream, ProtoMessage_fields, &msg);

        Serial.printf("%d|", msg.address);
        for (int i = 0; i < msg.data.size; i++) {
            Serial.printf("%02X", msg.data.bytes[i]);
            if (i != msg.data.size - 1) {
                Serial.printf(",");
            }
        }
        Serial.printf("|%d\n", msg.time);
    }
}
