// #include <Arduino.h>
// /**
//  * Test fonctionnement
//  */
// void setup()
// {
//   Serial.begin(9600);
//   pinMode(14, INPUT_PULLUP);
// }

// void loop()
// {
//   uint32_t tStart;
//   pinMode(14, OUTPUT);
//   digitalWrite(14, LOW);
//   delay(25); // 18 avec une grose marge
//   pinMode(14, HIGH);
//   pinMode(14, INPUT);
//   pinMode(14, INPUT_PULLUP);
//   float i = micros();
//   int total = 0;
//   while(micros() - i < 6000) {
//     Serial.println(digitalRead(14));
//     total++;
//   }
  
//   Serial.println("Run Complete");
//   Serial.println("Total bits: ");
//   Serial.print(total);
// }