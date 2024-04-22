#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncEventSource.h>
#include <RF24.h>
#include <WiFi.h>
#include <base64.h>
#include <pb_decode.h>

#include "CAN.pb.h"

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

// WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//==============================================================================
// nRF24L01+
//==============================================================================

void initNRF() {
    // if (radio.begin()) {
    //     Serial.println("Radio initialized successfully");
    // } else {
    //     Serial.println("Failed to initialize radio");
    // }

    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_MAX);

    radio.openReadingPipe(0, address);
    radio.startListening();
}

//==============================================================================
// WebSocket
//==============================================================================

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.println("Client connected");
        break;
    case WS_EVT_DISCONNECT:
        Serial.println("Client disconnected");
        break;
    case WS_EVT_ERROR:
        Serial.println("WebSocket error occurred");
        break;
    default:
        break;
    }
}

// void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
//     switch (type) {
//     case WS_EVT_CONNECT:
//         Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
//         break;
//     case WS_EVT_DISCONNECT:
//         Serial.printf("WebSocket client #%u disconnected\n", client->id());
//         break;
//     case WS_EVT_DATA:
//         // handleWebSocketMessage(arg, data, len);
//         break;
//     case WS_EVT_PONG:
//     case WS_EVT_ERROR:
//         break;
//     }
// }

//==============================================================================
// Setup
//==============================================================================

void setup() {
    Serial.begin(921600);
    delay(500);

    Serial.println();

    initNRF();

    // WiFi.softAP("GRC22-RLM");

    // IPAddress IP = WiFi.softAPIP();
    // Serial.print("AP IP address: ");
    // Serial.println(IP);

    // ws.onEvent(onEvent);
    // server.addHandler(&ws);
    // server.begin();
}

//==============================================================================
// Loop
//==============================================================================

// JsonDocument doc;

// uint32_t start;
// uint32_t delta;

void loop() {
    if (radio.available()) {
        radio.read(&nrf_buffer, sizeof(nrf_buffer));

        CAN msg = CAN_init_default;

        pb_istream_t input_stream = pb_istream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
        pb_decode(&input_stream, CAN_fields, &msg);

        Serial.printf("%X", msg.address);
        for (int i = 0; i < msg.data.size; i++) {
            Serial.printf(",%02X", msg.data.bytes[i]);
        }
        Serial.println();

        // doc["address"] = msg.address;

        // for (int i = 0; i < msg.data.size; i++) {
        //     doc["data"][i] = msg.data.bytes[i];
        // }

        // String json_string;
        // serializeJson(doc, json_string);

        // Serial.println(json_string);
    }
}
