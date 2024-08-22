#if defined(ESP8266)
#include <ESP8266WiFi.h>
#define THINGSBOARD_ENABLE_PROGMEM 0
#elif defined(ESP32) || defined(RASPBERRYPI_PICO) || defined(RASPBERRYPI_PICO_W)
#include <WiFi.h>
#endif
#include "attributes_led_tb.h"
#include "settings_led_tb.h"
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoardSized<Default_Fields_Amount, Default_Subscriptions_Amount, MAX_ATTRIBUTES> tb(mqttClient, MAX_MESSAGE_SIZE);

constexpr int16_t telemetrySendInterval = 2000U;
uint32_t previousDataSend;

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

const bool reconnect() {
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }
  InitWiFi();
  return true;
}

void processSetLedMode(const JsonVariantConst &data, JsonDocument &response) {
  Serial.println("Received the set led state RPC method");
  int new_mode = data;
  Serial.print("Mode to change: ");
  Serial.println(new_mode);
  StaticJsonDocument<1> response_doc;
  if (new_mode != 0 && new_mode != 1) {
    response_doc["error"] = "Unknown mode!";
    response.set(response_doc);
    return;
  }
  ledMode = new_mode;
  attributesChanged = true;
  response_doc["newMode"] = (int)ledMode;
  response.set(response_doc);
}

const std::array<RPC_Callback, 1U> callbacks = { 
  RPC_Callback{ "setLedMode", processSetLedMode }
};

void processSharedAttributes(const JsonObjectConst &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
      const uint16_t new_interval = it->value().as<uint16_t>();
      if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
        blinkingInterval = new_interval;
        Serial.print("Blinking interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
      ledState = it->value().as<bool>();
      if (LED_BLEUE != 99) {
        digitalWrite(LED_BLEUE, ledState);
      }
      Serial.print("LED state is set to: ");
      Serial.println(ledState);
    }
  }
  attributesChanged = true;
}

void processClientAttributes(const JsonObjectConst &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), LED_MODE_ATTR) == 0) {
      const uint16_t new_mode = it->value().as<uint16_t>();
      ledMode = new_mode;
    }
  }
}

const Shared_Attribute_Callback<MAX_ATTRIBUTES> attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback<MAX_ATTRIBUTES> attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback<MAX_ATTRIBUTES> attribute_client_request_callback(&processClientAttributes, CLIENT_ATTRIBUTES_LIST.cbegin(), CLIENT_ATTRIBUTES_LIST.cend());

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  if (LED_BLEUE != 99) {
    pinMode(LED_BLEUE, OUTPUT);
  }
  delay(1000);
  InitWiFi();
}

void loop() {
  delay(10);

  if (!reconnect()) {
    return;
  }

  if (!tb.connected()) {
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
    tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
    Serial.println("Subscribing for RPC...");
    if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }
    if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
      Serial.println("Failed to subscribe for shared attribute updates");
      return;
    }
    Serial.println("Subscribe done");
    if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
      Serial.println("Failed to request for shared attributes");
      return;
    }
    if (!tb.Client_Attributes_Request(attribute_client_request_callback)) {
      Serial.println("Failed to request for client attributes");
      return;
    }
  }

  if (attributesChanged) {
    attributesChanged = false;
    if (ledMode == 0) {
      previousStateChange = millis();
    }
    tb.sendTelemetryData(LED_MODE_ATTR, ledMode);
    tb.sendTelemetryData(LED_STATE_ATTR, ledState);
    tb.sendAttributeData(LED_MODE_ATTR, ledMode);
    tb.sendAttributeData(LED_STATE_ATTR, ledState);
  }

  if (ledMode == 1 && millis() - previousStateChange > blinkingInterval) {
    previousStateChange = millis();
    ledState = !ledState;
    tb.sendTelemetryData(LED_STATE_ATTR, ledState);
    tb.sendAttributeData(LED_STATE_ATTR, ledState);
    if (LED_BLEUE == 99) {
      Serial.print("LED state changed to: ");
      Serial.println(ledState);
    } else {
      digitalWrite(LED_BLEUE, ledState);
    }
  }

  if (millis() - previousDataSend > telemetrySendInterval) {
    previousDataSend = millis();
    tb.sendTelemetryData("temperature", random(10, 20));
    tb.sendAttributeData("rssi", WiFi.RSSI());
    tb.sendAttributeData("channel", WiFi.channel());
    tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
    tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
    tb.sendAttributeData("ssid", WiFi.SSID().c_str());
  }

  tb.loop();
}
