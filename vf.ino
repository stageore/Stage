#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "index.h"
//
const byte DNS_PORT = 53;
DNSServer dnsServer;

  
IPAddress apIP(192, 168, 1, 1);
ESP8266WebServer webServer(80);


#define PIN          0
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

/* Set these to your desired credentials. */
const char *ssid ="lampe-ORE-Nom-Prénom";
const char *password = "";
int r, g, b;

void setup() {
  EEPROM.begin(512);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password, 1);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
 // dnsServer.start(DNS_PORT, "lampe.app", appIP);


  webServer.on("/", []() {
    webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    String s = MAIN_page;
    webServer.send(200, "text/html", s); 
    webServer.client().stop();
  });
  
  webServer.on("/color", []() {
    webServer.send(200, "text/html", "OK");
    r = webServer.arg("r").toInt();
    g = webServer.arg("g").toInt();
    b = webServer.arg("b").toInt();  
    EEPROM.write(1, r);
    EEPROM.write(2, g);
    EEPROM.write(3, b);
    EEPROM.commit();
    webServer.client().stop();
  });

  webServer.onNotFound([]() {
    String message = "Hello World!\n\n";
    message += "URI: ";
    message += webServer.uri();
    webServer.send(200, "text/plain", message);
  });
  
  webServer.begin();
  
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(1, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(2, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(3, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(4, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(5, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(6, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(7, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.show();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  pixels.setPixelColor(0, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(1, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(2, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(3, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
    pixels.setPixelColor(4, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(5, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(6, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.setPixelColor(7, pixels.Color(EEPROM.read(1),EEPROM.read(2),EEPROM.read(3)));
  pixels.show();
}
