int redLedPin = 5;
int redBtnPin = 22;


void setup() {
    Serial.begin(9600);
    pinMode(redLedPin, OUTPUT);
    pinMode(redBtnPin, INPUT);
    Serial.println("Setup terminé");
}


void loop() {
    if (digitalRead(redBtnPin) == HIGH) {
        digitalWrite(redLedPin, HIGH);
        Serial.println("Allumée");
    } else {
        digitalWrite(redLedPin, LOW);
        Serial.println("Éteint");
    }
    delay(1000);
}
