#define RELAY_PIN 18

void setup() {
  // Initialiser le pin du relais comme sortie
  pinMode(RELAY_PIN, OUTPUT);
  
  // Désactiver le relais au départ
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {
  // Activer la pompe à eau
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000); // Garder la pompe activée pendant 5 secondes

  // Désactiver la pompe à eau
  digitalWrite(RELAY_PIN, LOW);
  delay(5000); // Garder la pompe désactivée pendant 5 secondes
}
