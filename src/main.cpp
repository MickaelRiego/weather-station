#include <Arduino.h>
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

void setup()
{
  Serial.begin(9600);

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
    result.humidite = (float)data[0];
    result.temperature = (float)data[2];
    result.isValid = true;
  } else {
    result.isValid = false;
  }

  return result;
}