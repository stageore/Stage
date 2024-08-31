#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configuration WiFi
const char* ssid = "Galaxy_S21";
const char* password = "12345678";

// Configuration MQTT pour ThingsBoard
const char* mqtt_server = "192.168.1.174";
const char* mqtt_token = "sqW1hsfNqLRAEkym3LKQ";

// Pins
const int soilSensorPin = 26; // ADC pin pour le capteur d'humidité
const int pumpPin = 18; // GPIO pour la pompe

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup() {
  Serial.begin(115200);

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convertir le payload en chaîne
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Utiliser ArduinoJson pour analyser le JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (!error) {
    const char* method = doc["method"];
    bool params = doc["params"];

    if (String(method) == "setValue") {
      if (params) {
        digitalWrite(pumpPin, HIGH);  // Activer la pompe
        Serial.println("Pump ON");
      } else {
        digitalWrite(pumpPin, LOW);   // Désactiver la pompe
        Serial.println("Pump OFF");
      }
    }
  } else {
    Serial.println("Failed to parse JSON");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_token, NULL)) {
      Serial.println("connected");
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

   unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    int sensorValue = analogRead(soilSensorPin);
    int humidity = sensorValue;

    // Check if humidity is below 50% and activate pump if necessary
    if (humidity < 50) {
      digitalWrite(pumpPin, HIGH);
      Serial.println("Pump ON");
    } else {
      digitalWrite(pumpPin, LOW);
      Serial.println("Pump OFF");
    }

    String payload = "{";
    payload += "\"humidity\":";
    payload += humidity;
    payload += "}";

    payload.toCharArray(msg, MSG_BUFFER_SIZE);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("v1/devices/me/telemetry", msg);
  }
}
