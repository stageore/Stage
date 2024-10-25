//Include les  2 autres fichier .h lié à ce dossier qui seront essentiel par la suite 
#include "pompe_jardin_settings.h"
#include "pompe_jardin_attributes.h"
#include "listes_fonctions.h"

void setup() {
  //On lance le moniteur série. 
  Serial.begin(115200);
  Serial.println("moniteur série");
  //On lance le BMP 
  unsigned status;
  status=bmp.begin(0x76);
  //On défini que l'on veut mesurer le courant dans le pin du capteur d'humidité
  pinMode(moisturePin,INPUT);
  // On défini que l'on a un message à envoyer au relais 
  pinMode(relayPin,OUTPUT);
  // init les ruban de LEDS imporants
  FastLED.addLeds<LED_TYPE, humstrippin, COLOR_ORDER>(led_humi, NUM_LED);
  FastLED.addLeds<LED_TYPE, temptrippin, COLOR_ORDER>(led_temp, NUM_LED);
  // On lance le Wi-Fi
  InitWiFi();
  delay(1000); //temps d'attente
}

void loop() {
  TBLancement();// On lance TB, voir listes fonctions
  getInfoAPI();//On prend les infos API , voir listes fonctions. 
  if (attributesChanged) { // si les attributs ont changés. 
    attributesChanged = false; // on repasse la variable en false.  
    //On envoie les valeurs télémétrie de nos attributs. 
    tb.sendTelemetryData(T_ARROSAGE_ATTR, t_arro);
    tb.sendTelemetryData(BTN_MANUEL_ATTR,btn_manuel);
    tb.sendTelemetryData(BTN_AUTO_ATTR,btn_auto);
    // On envoie les valeurs attributs de nos attributs. 
    tb.sendAttributeData(T_ARROSAGE_ATTR, t_arro);
    tb.sendAttributeData(BTN_MANUEL_ATTR,btn_manuel);
    tb.sendAttributeData(BTN_AUTO_ATTR,btn_auto);
    delay(1000);
  }
//On mesure la valeur que renvoie le capteur que l'on remap sur une échelle de  0 à 100
int moisture=map(analogRead(moisturePin),930,3630,100,0);
//on envoie un retour sur le moniteur série. 
Serial.print("Humi :");
Serial.println(moisture);
  if (btn_auto==true){// si mode automatique actif 
    Serial.println("mode auto actif");
    if ((moisture <= humidityRate) && (temp_J1 < 30)) { // l'humidité est en dessous de la limite fixé dans settings et la température est en dessous de 30°. 
      Serial.println("arrosage automatique en cours ");
      digitalWrite(relayPin,HIGH); /// on allume l'arrosage pour une durée  t_arro
      delay(t_arro);
      digitalWrite(relayPin,LOW);
      btn_manuel=false;
    }
    else { // si l'humidité est bonne alors on laisse la pompe éteinte. 
    digitalWrite(relayPin,LOW);
    btn_manuel=false;
    }
  }
  else { // si le mode auto est sur false.  
  Serial.println("mode manuel actif");
  if (btn_manuel==true){ // et que le btn d'arosage manuel est appuyé
    Serial.println("arrosage manuel en cours ");
    digitalWrite(relayPin,HIGH); // on lance l'arrosage pour une durrée t_arro
    delay(t_arro); 
    digitalWrite(relayPin,LOW);
    btn_manuel=false;
  }
  else {// si le btn manuel  n'est pas  activé alors on éteint la pompe. 
    digitalWrite(relayPin,LOW);
  }
  }

  // On prend toute les mesures utile  du BMP. 
  float temp=bmp.readTemperature();
  float alt=bmp.readAltitude(1000);
  float press=bmp.readPressure()/100;
  ledScenario();
  
  if (millis() - previousDataSend > telemetrySendInterval) {// si le temps depuis le dernier envoi de télémétrie est plus grand que la  valeur qu'on a fixé. 
    
    previousDataSend = millis();
      //On envoie nos valeurs  sur TB comme telemetry.  
    tb.sendTelemetryData("temperature",temp);
    tb.sendTelemetryData("pression",press);
    tb.sendTelemetryData("altitude",alt);
    tb.sendTelemetryData("humidite",moisture);
    tb.sendTelemetryData("btn_auto",btn_auto);
    tb.sendTelemetryData("btn_manuel",btn_manuel);
    // On envoie alors les valeurs de télémétrie des proba de pluies pour les  5 prochains jours. 
    tb.sendTelemetryData("pluie_jour_1",pop_J1);
    tb.sendTelemetryData("pluie_jour_2",pop_J2);
    tb.sendTelemetryData("pluie_jour_3",pop_J3);
    tb.sendTelemetryData("pluie_jour_4",pop_J4);
    tb.sendTelemetryData("pluie_jour_5",pop_J5);
    // On envoie alors les valeurs de télémétrie de la température envoyé par l'API
    tb.sendTelemetryData("temperature_jour_1",temp_J1);
    tb.sendTelemetryData("temperature_jour_2",temp_J2);
    tb.sendTelemetryData("temperature_jour_3",temp_J3);
    tb.sendTelemetryData("temperature_jour_4",temp_J4);
    tb.sendTelemetryData("temperature_jour_5",temp_J5);
    // et comme attribut pour celle lié au 
    tb.sendAttributeData("rssi", WiFi.RSSI());
    tb.sendAttributeData("channel", WiFi.channel());
    tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
    tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
    tb.sendAttributeData("ssid", WiFi.SSID().c_str());
  }
  
  delay(1000);
  tb.loop();
}
