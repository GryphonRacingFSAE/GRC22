#include <Arduino.h>
#include <RF24.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <mbedtls/base64.h>

#include "CAN.pb.h"

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

const char* ssid = "GRC22-RLM";
const char* password = "burnt accumulator";

AsyncWebServer server(8765);
AsyncWebSocket ws("/ws");

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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

    Serial.println("\nInitializing radio...");
    // radio.begin();
    // radio.openReadingPipe(0, address);
    // radio.startListening();

    WiFi.softAP(ssid, password);

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

void loop() {
    CAN msg = CAN_init_default;
    msg.address = 0x177;
    msg.data.size = 8;
    msg.data.bytes[7] = 0x72;


    uint8_t buffer[128];
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&ostream, CAN_fields, &msg)) {
        Serial.println("Failed to encode");
        return;
    }
    
    Serial.printf("bytes written: %d\n", ostream.bytes_written);

    // radio.read(&buffer, sizeof(buffer));

    pb_istream_t stream = pb_istream_from_buffer(buffer, sizeof(buffer));
    pb_decode(&stream, CAN_fields, &msg);
    Serial.printf("bytes left: %d\n", stream.bytes_left);
    

    Serial.printf("Message: 0x%08x, DLC: %d, Data: ",
                    msg.address,
                    msg.data.size);

    for (uint32_t i = 0; i < msg.data.size; i++) {
        Serial.printf("0x%02x, ", msg.data.bytes[i]);
    }

    Serial.print("\r\n");

    // Encode protobuf into base64 encoded string
    uint8_t encoded_can_message[CAN_size];
    pb_ostream_t encoder_ostream = pb_ostream_from_buffer(encoded_can_message, sizeof(encoded_can_message));
    if (!pb_encode(&encoder_ostream, CAN_fields, &msg)) {
        Serial.println("Failed to encode");
        return;
    }

    size_t bytes_written_for_serialization = encoder_ostream.bytes_written;
    Serial.printf("Serialization bytes written: %d\n", bytes_written_for_serialization);

    unsigned char base64_message[24];
    size_t outlen;

    mbedtls_base64_encode(base64_message, 24, &outlen, encoded_can_message, bytes_written_for_serialization);
    ws.textAll((char*)base64_message);
    delay(100);
}

