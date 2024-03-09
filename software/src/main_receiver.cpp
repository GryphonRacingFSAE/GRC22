#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <RF24.h>
#include <WiFi.h>
#include <mbedtls/base64.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "CAN.pb.h"

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

const char* ssid = "GRC22-RLM";
const char* password = "burnt accumulator";

AsyncWebServer server(8765);
AsyncWebSocket ws("/ws");

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        // handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();

    if (radio.begin()) {
        Serial.println("Radio initialized successfully");
    } else {
        Serial.println("Failed to initialize radio");
    }
    radio.openReadingPipe(0, address);
    radio.startListening();

    WiFi.softAP(ssid);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     request->send(SPIFFS, "/index.html", "text/html");
    // });

    server.begin();
}

uint8_t nrf_buffer[128];

void loop() {
    if (radio.available()) {
        radio.read(&nrf_buffer, sizeof(nrf_buffer));

        CAN msg = CAN_init_default;

        pb_istream_t input_stream = pb_istream_from_buffer(nrf_buffer, sizeof(nrf_buffer));
        pb_decode(&input_stream, CAN_fields, &msg);

        Serial.printf("[0x%X] ", msg.address);
        for (int i = 0; i < msg.data.size; i++) {
            Serial.printf("%02X ", msg.data.bytes[i]);
        }
        Serial.println();

        // Encode protobuf into base64 encoded string
        uint8_t encoded_can_message[128];
        pb_ostream_t encoder_ostream = pb_ostream_from_buffer(encoded_can_message, sizeof(encoded_can_message));
        if (!pb_encode(&encoder_ostream, CAN_fields, &msg)) {
            Serial.println("Failed to encode");
            return;
        }

        size_t bytes_written_for_serialization = encoder_ostream.bytes_written;
        Serial.printf("Serialization bytes written: %d\n", bytes_written_for_serialization);

        unsigned char base64_message[128] = {};
        size_t outlen;

        mbedtls_base64_encode(base64_message, 128, &outlen, encoded_can_message, bytes_written_for_serialization);
        ws.textAll((char*)base64_message);
        delay(100);
    }
}
