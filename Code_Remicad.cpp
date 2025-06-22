#include <Wire.h>
#include "rgb_lcd.h"
#include <DHT.h>


// Définition des broches
#define DHTPIN A1               // DHT11 sur A1
#define LDRPIN A2               // LDR sur A2
#define TRIGPIN 8               // HC-SR04 Trig sur D8
#define ECHOPIN 9               // HC-SR04 Echo sur D9
#define DHTTYPE DHT11           // Type de capteur

// Paramètres LDR (calibrés pour lux)
const float R = 10000.0;        // Résistance fixe (10kΩ)
const float GAMMA = 0.7;        // Coefficient gamma
const float RL10 = 50.0;        // Résistance à 10 lux (kΩ)

DHT dht(DHTPIN, DHTTYPE);       // Initialisation du DHT11
rgb_lcd lcd;                    // Initialisation de l'écran LCD

unsigned long previousMillis = 0;
const long interval = 2000;     // Intervalle de 2 secondes
int displayState = 0;           // 0: Température, 1: Humidité, 2: Luminosité, 3: Distance

// Fonction de conversion LDR → lux
float readLux() {
  int raw = analogRead(LDRPIN);
  // Inversion du signal si nécessaire (selon votre branchement)
  raw = 4095 - raw; // À ajouter SI les valeurs augmentent dans le noir

  float Vout = raw * (3.3 / 4095.0);       // Conversion en tension (GIGA R1 = ADC 12-bit)
  float RLDR = (3.3 * R) / Vout - R;       // Résistance LDR (Ω)
  RLDR = RLDR / 1000.0;                    // Conversion en kΩ
  float lux = pow(RL10 / RLDR, 1/GAMMA) * 10; // Formule standard
  return lux;
}

// Fonction pour mesurer la distance (HC-SR04)
float readDistance() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  long duration = pulseIn(ECHOPIN, HIGH);
  return duration * 0.034 / 2; // Conversion en cm (vitesse du son = 340 m/s)
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(TRIGPIN, OUTPUT);    // Config Trig en sortie
  pinMode(ECHOPIN, INPUT);     // Config Echo en entrée
  lcd.begin(16, 2);
  lcd.setRGB(0, 128, 255);     // Fond bleu clair
  lcd.print("Initialisation...");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Lecture des capteurs
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float lux = readLux();
    float distance = readDistance();       // Lecture distance (cm)

    // Affichage cyclique sur LCD
    lcd.clear();
    lcd.setCursor(0, 0);

    switch (displayState) {
      case 0:  // Température
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
        lcd.print(temperature);
        lcd.print("\xDF""C");
        displayState = 1;
        break;

      case 1:  // Humidité
        lcd.print("Humidite:");
        lcd.setCursor(0, 1);
        lcd.print(humidity);
        lcd.print("%");
        displayState = 2;
        break;

      case 2:  // Luminosité
        lcd.print("Luminosite:");
        lcd.setCursor(0, 1);
        lcd.print(lux, 1);      // 1 décimale
        lcd.print(" lux");
        displayState = 3;
        break;

      case 3:  // Distance
        lcd.print("Distance:");
        lcd.setCursor(0, 1);
        lcd.print(distance, 1); // 1 décimale
        lcd.print("cm");
        displayState = 0;
        break;
    }

    // Debug sur moniteur série
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("°C | Hum: ");
    Serial.print(humidity);
    Serial.print("% | Lumiere: ");
    Serial.print(lux, 1);
    Serial.print(" lux | Dist: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }
}
