// Fonction qui initie la connection Wifi et permet d'avoir un retour visuel niveau moniteur série pour son avancement. 
void InitWiFi() {
  Serial.println("Tentative de connection au Wi-Fi ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);/// lance le wifi 
  while (WiFi.status() != WL_CONNECTED) {// tant que le statut n'est pas connecté on print un pt toute le demi seconde.  
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connecte au Wi-Fi");
}
//------------------------------------------------------------------------------
// Fonction de reconnect si la carte detecte qu'elle a été déconnctée. 
const bool reconnect() {
  // Teste que l'on a pas  été connecté dans une variable status. 
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {// si le status est connecté 
    return true; //return faire sortir de la fonction sans traiter les chose après le IF
  }
  //Si if pas vérifié alors on relance InitWiFi
  InitWiFi();
  return true;
}
//------------------------------------------------------------------------------
void processSetBtnManuel(const JsonVariantConst &data, JsonDocument &response) {
  Serial.println("Reception etat du bouton manuel via methode RPC");
  // Stocke la donnée reçue comme "nouveau mode"
  int new_mode = data;

  Serial.print("Mode du bouton changer : ");
  Serial.println(new_mode);
  StaticJsonDocument<1> response_doc;

  if (new_mode != 0 && new_mode != 1) { /// Si le mode n'est pas un entier soit 0 ou 1 (booléen true/false) alors on a une erreur
    response_doc["error"] = "Mode Inconnu!";
    response.set(response_doc);
    return; // si erreur il y a on sors imédiatement de la fonction
  }
  //Si pas d'erreur on met à jour l'état du bouton de l'état manuel.
  btn_manuel = new_mode;

  //On stocke que l'on a bien changer un attribut.  
  attributesChanged = true;

  // On renvoi l'état actuel du bouton.  
  response_doc["newMode"] = (int)btn_manuel;
  response.set(response_doc);
}
//------------------------------------------------------------------------------
void processSetBtnAuto(const JsonVariantConst &data, JsonDocument &response) {

  Serial.println("Reception etat du bouton mode auto  via methode RPC");
  // Stocke la donnée reçue comme "nouveau mode"
  int new_mode = data;

  Serial.print("Mode modifier vers : ");
  Serial.println(new_mode);
  StaticJsonDocument<1> response_doc;

  if (new_mode != 0 && new_mode != 1) { /// Si le mode n'est pas un entier soit 0 ou 1 (booléen true/false) alors on a une erreur
    response_doc["error"] = "Mode Inconnu!";
    response.set(response_doc);
    return; // si erreur il y a on sors imédiatement de la fonction
  }
  //Si pas d'erreur on met à jour l'état du bouton de l'état manuel.
  btn_auto = new_mode;

  //On stocke que l'on a bien changer un attribut.  
  attributesChanged = true;

  // On renvoi l'état actuel du bouton.  
  response_doc["newMode"] = (int)btn_auto;
  response.set(response_doc);
}
//------------------------------------------------------------------------------
// Définition des fonction RPC qui seront utile dans TB par la suite
const std::array<RPC_Callback, 2U> callbacks= {
  RPC_Callback{"setBtnAuto",processSetBtnAuto},
  RPC_Callback{"setBtnManuel",processSetBtnManuel}
  };
  // la partie entre "" correspond au nom qu'ils auront pour TB 
  // la partie après correspond au nom de la fonction dans le code arduino
//------------------------------------------------------------------------------
void processSharedAttributes(const JsonObjectConst &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    // cette condition permet de verifier si 2 chaines de caractère entre l'input 
    //et la valeur stocké dans la carte sont identique.
    if (strcmp(it->key().c_str(), T_ARROSAGE_ATTR) == 0) {
      //Si ce n'est pas le cas avec on prendre la chaine de caractère input 
      //et on le met sont la forme d'un entier qu'on pourra traiter
      const uint16_t new_interval = it->value().as<uint16_t>();
      //Si le nouvel intervalle est entre les valeurs min et max autorisé.  
      if (new_interval >= T_ARROSAGE_MS_MIN && new_interval <= T_ARROSAGE_MS_MAX) {
        // alors on def le temps d'arrosage comme le nouveau intervalle et on le comunique sur le moniteur série. 
        t_arro = new_interval;
        Serial.print("Temps arrosage modifier : ");
        Serial.print(new_interval);
        Serial.println(" ms");
      }
    // même principe de comparasion 
    } else if (strcmp(it->key().c_str(), BTN_MANUEL_ATTR) == 0) {
      //si y'a modification alors on  stocke le nouvel état dans la variable du btn manuel. 
      btn_manuel = it->value().as<bool>();
      Serial.print("Changement état bouton manuel: ");
      Serial.println(btn_manuel );
      
    } 
    //Exactement le même else if pour l'autre bouton. 
    else if (strcmp(it->key().c_str(), BTN_AUTO_ATTR) == 0) {
      btn_auto = it->value().as<bool>();
      Serial.print("Changement état bouton manuel: ");
      Serial.println(btn_auto );
    }
  }
  attributesChanged = true; // On  stocke que l'on a changer les  attributs
}
//------------------------------------------------------------------------------
//Definition des callback pour  les atttributs partagé (on peut aussi pour les autres attributs: client mais nous n'en avons pas ici )
const Shared_Attribute_Callback<MAX_ATTRIBUTES> attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback<MAX_ATTRIBUTES> attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
// ------------------------------------------------------------------------------
void TBLancement (){
  if (!tb.connected()) {
    // Connexion à  TB 
    Serial.print("Connexion a : ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" avec le device  ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) { // Si tb connect n'est tjr pas true.  
      Serial.println("Echec de la connexion");
      return;
    }
    // Si on a pu se connecter alors on  envoie notre adress max en  attribute. 
    tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());

    Serial.println("Subscribing for RPC...");
    //On va ici faire bcp de conditions afin de sortir des message d'erreur précis pour le debug.  
    if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {// Si il y a un echec pour les fonction RPCs. 
      Serial.println("Failed to subscribe for RPC");
      return;
    }
  if (!tb.Shared_Attributes_Subscribe(attributes_callback)) { // Si il y a un échec pour les attributs partagés. 
      Serial.println("Failed to subscribe for shared attribute updates");
      return;
    }

    Serial.println("Inscription réussie");

    // Si on ne peut pas demander l'état actuel des attributs partagés. 
    if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
      Serial.println("Failed to request for shared attributes");
      return;
    }
  }
}
// ------------------------------------------------------------------------------

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // On lance la  connexion au site avec le  url donné à la fonction et le wifi client défini plus haut. 
  http.begin(client, serverName);
  
  // On demande une réponse  au serveur HTTP
  int httpResponseCode = http.GET();
  // On défini le payload qui sera notre  sorte de fonction comme une string vide. 
  String payload = "{}"; 
  
  if (httpResponseCode>0) {// Si la réponse du  serveur est pas nulle 
    Serial.print("HTTP Response code: ");// on print la réponse dans le serial
    Serial.println(httpResponseCode);
    payload = http.getString();// On la stocke dans notre payload
  }
  else {// sinon on envoie un message d'erreur. 
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // On fini les demande au serveur HTTP
  http.end();

  // la fonction ressort ce que le serveur HTTP à donné. 
  return payload;
}
// ------------------------------------------------------------------------------
void getInfoAPI() {
  if ((millis() - lastTime) > timerDelay) { // si le délai entre demandes API est dépassé
    if (WiFi.status() == WL_CONNECTED) {
      jsonBuffer = httpGETRequest(url.c_str()); // on met dans le buffer la réponse de l'API
      Serial.println(jsonBuffer); // on l'imprime dans le moniteur série
      
      DynamicJsonDocument myObject(2048); 
      deserializeJson(myObject, jsonBuffer);

      // Variables pour les probabilités de pluie, température et vitesse du vent pour chaque jour
      float p_J1 = 1.0, windSum_J1 = 0, tempSum_J1 = 0;
      for (int i = 0; i < 8; i++) {
        float buff = float(myObject["list"][i]["pop"]);
        p_J1 *= (1 - buff);
        tempSum_J1 += float(myObject["list"][i]["main"]["temp"]);
        windSum_J1 += float(myObject["list"][i]["wind"]["speed"]) * 3.6; // Conversion en km/h
      }
      pop_J1 = (1 - p_J1) * 100;
      temp_J1 = (tempSum_J1 / 8) - 273.15; // Conversion en Celsius
      wind_J1 = windSum_J1 / 8;  // Vitesse moyenne du vent pour le jour 1

      float p_J2 = 1.0, windSum_J2 = 0, tempSum_J2 = 0;
      for (int i = 8; i < 16; i++) {
        float buff = float(myObject["list"][i]["pop"]);
        p_J2 *= (1 - buff);
        tempSum_J2 += float(myObject["list"][i]["main"]["temp"]);
        windSum_J2 += float(myObject["list"][i]["wind"]["speed"]) * 3.6; // Conversion en km/h
      }
      pop_J2 = (1 - p_J2) * 100;
      temp_J2 = (tempSum_J2 / 8) - 273.15;
      wind_J2 = windSum_J2 / 8;

      float p_J3 = 1.0, windSum_J3 = 0, tempSum_J3 = 0;
      for (int i = 16; i < 24; i++) {
        float buff = float(myObject["list"][i]["pop"]);
        p_J3 *= (1 - buff);
        tempSum_J3 += float(myObject["list"][i]["main"]["temp"]);
        windSum_J3 += float(myObject["list"][i]["wind"]["speed"]) * 3.6; // Conversion en km/h
      }
      pop_J3 = (1 - p_J3) * 100;
      temp_J3 = (tempSum_J3 / 8) - 273.15;
      wind_J3 = windSum_J3 / 8;

      float p_J4 = 1.0, windSum_J4 = 0, tempSum_J4 = 0;
      for (int i = 24; i < 32; i++) {
        float buff = float(myObject["list"][i]["pop"]);
        p_J4 *= (1 - buff);
        tempSum_J4 += float(myObject["list"][i]["main"]["temp"]);
        windSum_J4 += float(myObject["list"][i]["wind"]["speed"]) * 3.6; // Conversion en km/h
      }
      pop_J4 = (1 - p_J4) * 100;
      temp_J4 = (tempSum_J4 / 8) - 273.15;
      wind_J4 = windSum_J4 / 8;

      float p_J5 = 1.0, windSum_J5 = 0, tempSum_J5 = 0;
      for (int i = 32; i < 40; i++) {
        float buff = float(myObject["list"][i]["pop"]);
        p_J5 *= (1 - buff);
        tempSum_J5 += float(myObject["list"][i]["main"]["temp"]);
        windSum_J5 += float(myObject["list"][i]["wind"]["speed"]) * 3.6; // Conversion en km/h
      }
      pop_J5 = (1 - p_J5) * 100;
      temp_J5 = (tempSum_J5 / 8) - 273.15;
      wind_J5 = windSum_J5 / 8;

      // Affichage des données dans le moniteur série
      Serial.print("POP Jour 1: ");
      Serial.println(pop_J1);
      Serial.print("Température Jour 1: ");
      Serial.println(temp_J1);
      Serial.print("Vitesse du vent Jour 1 (km/h): ");
      Serial.println(wind_J1);

      Serial.print("POP Jour 2: ");
      Serial.println(pop_J2);
      Serial.print("Température Jour 2: ");
      Serial.println(temp_J2);
      Serial.print("Vitesse du vent Jour 2 (km/h): ");
      Serial.println(wind_J2);

      Serial.print("POP Jour 3: ");
      Serial.println(pop_J3);
      Serial.print("Température Jour 3: ");
      Serial.println(temp_J3);
      Serial.print("Vitesse du vent Jour 3 (km/h): ");
      Serial.println(wind_J3);

      Serial.print("POP Jour 4: ");
      Serial.println(pop_J4);
      Serial.print("Température Jour 4: ");
      Serial.println(temp_J4);
      Serial.print("Vitesse du vent Jour 4 (km/h): ");
      Serial.println(wind_J4);

      Serial.print("POP Jour 5: ");
      Serial.println(pop_J5);
      Serial.print("Température Jour 5: ");
      Serial.println(temp_J5);
      Serial.print("Vitesse du vent Jour 5 (km/h): ");
      Serial.println(wind_J5);

    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis(); // Réinitialisation de lastTime
  }
}




// ------------------------------------------------------------------------------
void ledScenario(){
  // On redéfinie nos moisture et  temperature  en début de fonction. 
  float temp=bmp.readTemperature();
  int moisture=map(analogRead(moisturePin),930,3630,100,0);

  // on faire un if qui change l'état des led tout les 20% d'humidité. 
   if (moisture<20){
    fill_solid(led_humi,NUM_LED,CRGB::AliceBlue); // fil solide remplie le ruban sur le nombre de led donné par la couleur donnée, 
    FastLED.show();// Permet de faire afficher ce changement. 
    // tout les autres boucle sur l'humidité et la temperature fonctionne sur le même principe (avec l'autre ruban pour la temperature.)
    
  }
  else if (moisture<40){
    fill_solid(led_humi,NUM_LED,CRGB::Aqua);
    FastLED.show();
  }
  else if (moisture<60){
    fill_solid(led_humi,NUM_LED,CRGB::Green);
    FastLED.show();
  }
  else if (moisture<80){
    fill_solid(led_humi,NUM_LED,CRGB::Purple);
    FastLED.show();
  }
  else{
    fill_solid(led_humi,NUM_LED,CRGB::DarkBlue);
    FastLED.show();
  }
  //changement de la couleur de la led en fonction de la température
  if (temp<30){
    fill_solid(led_temp,NUM_LED,CRGB::Yellow);
    FastLED.show();
  }
  else {
    fill_solid(led_temp,NUM_LED,CRGB::Red);
    FastLED.show();
  }
}
