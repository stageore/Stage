#include <DHT.h>
#define DHTPIN 18   // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11


// Initialisation du capteur DHT
DHT dht11(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(9600);
  dht11.begin();
}


void loop() {
  // Lecture des données du capteur
  float h = dht11.readHumidity();
  float t = dht11.readTemperature();


  // Vérification si des erreurs se sont produites lors de la lecture
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
   
  } else {
    // Affichage des valeurs sur le moniteur série
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));
  }


  delay(2000); // Attendre 2 secondes entre chaque mesure
}
