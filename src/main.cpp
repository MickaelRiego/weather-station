#include <Arduino.h>
#include <SPI.h>
/*
  DHT11 Communication Process
  Arduino Pull Low at least 18ms, then Arduino Pause 20-40microseconds for DHT's response
*/

struct DhtData {
  float temperature;
  float humidite;
  bool isValid; // Checksum
};

DhtData readDHT11(int pin);
uint8_t PIN_DHT11 = 14;
uint8_t PIN_DC = 32;
uint8_t PIN_CS = 26;
uint8_t PIN_BUSY = 25;
uint8_t PIN_RST = 33;

void sendCommand(uint8_t command)  {
  pinMode(PIN_DC, OUTPUT);
  digitalWrite(PIN_DC, LOW);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS,LOW);
  SPI.transfer(command);
  digitalWrite(PIN_CS, HIGH);
}

void sendData(uint8_t data) {
  pinMode(PIN_DC, OUTPUT);
  digitalWrite(PIN_DC, HIGH);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS,LOW);
  SPI.transfer(data);
  digitalWrite(PIN_CS, HIGH);
}

void waitBusy() {
  pinMode(PIN_BUSY, INPUT);
  while(digitalRead(PIN_BUSY) == HIGH) {
    delay(10);
  }
}

void initDisplay() {
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);
  delay(200); 
  digitalWrite(PIN_RST, HIGH);
  waitBusy();

  // Configurer la taille physique (Page 8 datasheet)
  sendCommand(0x01); 
  sendData(0xC7); // 199 Low Byte
  sendData(0x00); // High Byte
  sendData(0x00); // GD=0, SM=0, TB=0 

  // Mode d'entrée des données (Page 9)
  sendCommand(0x11);
  sendData(0x03); // X increment, Y increment

  // Définir la zone X (0 à 24) (Page 12)
  sendCommand(0x44);
  sendData(0x00); // Start
  sendData(0x18); // End (24 octets) 

  // Définir la zone Y (0 à 199) (Page 12)
  sendCommand(0x45); 
  sendData(0x00); // Start Low
  sendData(0x00); // Start High
  sendData(0xC7); // End Low 
  sendData(0x00); // End High

  // Option de mise à jour
  sendCommand(0x22); 
  sendData(0xF7); 
  
  sendCommand(0x20);

  waitBusy();
}

void clearScreen() {
  waitBusy();
  sendCommand(0x24); // Write RAM
  // (25 octets * 200 lignes)
  for (u_int32_t i=0; i<5000; i++) {
    sendData(0x00); // Pixen noir
  }
  sendCommand(0x22);
  sendData(0xF7);
  sendCommand(0x20);
  waitBusy();
}



void setup()
{
  Serial.begin(9600);
  SPI.begin();

  // Initialisation des broches
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_BUSY, INPUT);

  Serial.println("Initialisation de l'ecran...");
  initDisplay();
  
  Serial.println("Nettoyage de l'ecran (Noir)...");
  clearScreen();
  Serial.println("Termine !");

}

void loop()
{
  DhtData datas = readDHT11(14);

  if (datas.isValid) {
    Serial.print("Temp: ");
    Serial.print(datas.temperature);
    Serial.print("°C | Hum: ");
    Serial.print(datas.humidite);
    Serial.println("%");
  } else {
    Serial.println("Erreur : Données corrompues (Checksum)");
  }

  delay(2000); // on attend un peu, car dht11 un peu lent
}

DhtData readDHT11(int pin) {
  unsigned long timeout;
  DhtData result = {0, 0, false};
  uint8_t data[5] = {0, 0, 0, 0, 0};
  /**
   * 0: Hum int, 1: Hum decimal
   * 2: T int, 3: T decimal
   * 4: checksum
   * 8bit chaque
   */

  
  // signal départ
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20); // 18ms avec marge
  digitalWrite(pin, HIGH);
  delayMicroseconds(30); // 20-40 us
  pinMode(pin, INPUT_PULLUP);

  // attente réponse
  timeout = micros();
  while(digitalRead(pin) == HIGH) {
      if (micros() - timeout > 500) return result; // Si ça dure plus de 500µs, on abandonne
  }
  timeout = micros();
  while(digitalRead(pin) == LOW) {
      if (micros() - timeout > 500) return result;
  }
  timeout = micros();
  while(digitalRead(pin) == HIGH) {
      if (micros() - timeout > 500) return result;
  }

  // lecture des 40 bit
  for (int i = 0; i < 40; i++) {
    timeout = micros();
    while(digitalRead(pin) == LOW) {
        if (micros() - timeout > 500) return result;
    }
    unsigned long start = micros();
    timeout = micros();
    while(digitalRead(pin) == HIGH) {
        if (micros() - timeout > 500) return result;
    }

    if ((micros() - start) > 40)  { // si >40 us on considère 1 sinon 0
      data[i / 8] |=  (1 << (7 - (i % 8)));
    }
  }

  // verif checksum
  // somme 4 premiers octets doit être égale au 5ème
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) { // 0xFF force a être un octect la somme
    result.humidite = (float)data[0] + (float)data[1] / 10.0f;
    result.temperature = (float)data[2] + (float)data[3] / 10.0f; // On divise par 10 car dht11 envoie sous forme d'entier la decimal
    result.isValid = true;
  } else {
    result.isValid = false;
  }

  return result;
}
