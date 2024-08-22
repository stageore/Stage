#include <WiFi.h>
#include <PubSubClient.h>
#include "dht_settings_tb.h"
#include "dht_sensors_tb.h"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  initSensors();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the internet!");

  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32_Client", TOKEN, "")) {
      Serial.println("Connected to MQTT!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  float h = readHumidity();
  float t = readTemperature();

  if (!isnan(h) && !isnan(t)) {
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"temperature\":%.2f,\"humidity\":%.2f}", t, h);
    client.publish("v1/devices/me/telemetry", payload);
    Serial.println("Data published!");
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  client.loop();
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages
}
