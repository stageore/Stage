// Pin du capteur d'humidité
const int soilSensorPin = 26;  //Pin analogique obligatoire

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Lire la valeur du capteur d'humidité
  int soilMoistureValue = analogRead(soilSensorPin);

  int  moisture= map(soilMoistureValue,1000,3000,100,0); //conversion de l'entier codé sur 12bit en pourcentage
  // Afficher la valeur dans le terminal
  Serial.print("Soil Moisture: "); 
  Serial.println(moisture);
  Serial.println("%");
  
  delay(2000); // Attendre 2 secondes avant la prochaine lecture
}
