#ifndef SENSORS_H
#define SENSORS_H

#include <DHT.h>

#define DHTPIN 18
#define DHTTYPE DHT11

DHT dht11(DHTPIN, DHTTYPE);

void initSensors() {
  dht11.begin();
}

float readHumidity() {
  return dht11.readHumidity();
}

float readTemperature() {
  return dht11.readTemperature();
}

#endif
