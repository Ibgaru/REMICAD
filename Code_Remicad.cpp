#include <Wire.h>
#include "rgb_lcd.h"
#include <DHT.h>
#include <SPI.h>
#include <LoRa.h>


#define LORA_SS 10
#define LORA_RST 7
#define LORA_DIO0 2


#define DHTPIN A1
#define LDRPIN A2
#define TRIGPIN 8
#define ECHOPIN 9
#define DHTTYPE DHT11

const float R = 10000.0;
const float GAMMA = 0.7;
const float RL10 = 50.0;

DHT dht(DHTPIN, DHTTYPE);
rgb_lcd lcd;

unsigned long previousMillis = 0;
const long interval = 2000;
int displayState = 0;

float readLux() {
  int raw = analogRead(LDRPIN);
  raw = 4095 - raw;

  float Vout = raw * (3.3 / 4095.0);
  float RLDR = (3.3 * R) / Vout - R;
  RLDR = RLDR / 1000.0;
  float lux = pow(RL10 / RLDR, 1 / GAMMA) * 10;
  return lux;
}

float readDistance() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  long duration = pulseIn(ECHOPIN, HIGH);
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  lcd.begin(16, 2);
  lcd.setRGB(0, 128, 255);
  lcd.print("Initialisation...");
  delay(2000);
  lcd.clear();


  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(868E6)) {  
    Serial.println("Erreur initialisation LoRa!");
    while (1);
  }
  Serial.println("LoRa initialisé.");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float lux = readLux();
    float distance = readDistance();

    lcd.clear();
    lcd.setCursor(0, 0);

    switch (displayState) {
      case 0:
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
        lcd.print(temperature);
        lcd.print("\xDF""C");
        displayState = 1;
        break;
      case 1:
        lcd.print("Humidite:");
        lcd.setCursor(0, 1);
        lcd.print(humidity);
        lcd.print("%");
        displayState = 2;
        break;
      case 2:
        lcd.print("Luminosite:");
        lcd.setCursor(0, 1);
        lcd.print(lux, 1);
        lcd.print(" lux");
        displayState = 3;
        break;
      case 3:
        lcd.print("Distance:");
        lcd.setCursor(0, 1);
        lcd.print(distance, 1);
        lcd.print("cm");
        displayState = 0;
        break;
    }

    
    String data = String("T:") + temperature + 
                  ";H:" + humidity + 
                  ";L:" + lux + 
                  ";D:" + distance;

    
    LoRa.beginPacket();
    LoRa.print(data);
    LoRa.endPacket();

    
    Serial.println("Envoi LoRa : " + data);
  }
}
