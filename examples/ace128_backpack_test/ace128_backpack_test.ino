// include types & constants of Wiring core API
#include <Arduino.h>
#include <Wire.h>

#define RED_LED 10
#define GRN_LED 11
#define PCF_0 2
#define PCF_1 3
#define PCF_2 4
#define PCF_3 5
#define PCF_4 6
#define PCF_5 7
#define PCF_6 8
#define PCF_7 9

int ACE_ADDR = 0x20;

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(PCF_0, OUTPUT);
  pinMode(PCF_1, OUTPUT);
  pinMode(PCF_2, OUTPUT);
  pinMode(PCF_3, OUTPUT);
  pinMode(PCF_4, OUTPUT);
  pinMode(PCF_5, OUTPUT);
  pinMode(PCF_6, OUTPUT);
  pinMode(PCF_7, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GRN_LED, LOW);
  digitalWrite(PCF_0, LOW);
  digitalWrite(PCF_1, LOW);
  digitalWrite(PCF_2, LOW);
  digitalWrite(PCF_3, LOW);
  digitalWrite(PCF_4, LOW);
  digitalWrite(PCF_5, LOW);
  digitalWrite(PCF_6, LOW);
  digitalWrite(PCF_7, LOW);

  Serial.begin(9600);
  int error = 1;
  Wire.begin();
  while (error != 0) {
    ACE_ADDR = (ACE_ADDR == 0x20 ? 0x38 : 0x20);
    Serial.println("looking for backpack");
    Wire.beginTransmission(ACE_ADDR);
    error = Wire.endTransmission();
  }
  Serial.print("found backpack at ");
  Serial.println(ACE_ADDR, HEX);

  for (int i = 0; i <= 255; i++) {
    digitalWrite(PCF_0, i & 0x01 ? HIGH : LOW);
    digitalWrite(PCF_1, i & 0x02 ? HIGH : LOW);
    digitalWrite(PCF_2, i & 0x04 ? HIGH : LOW);
    digitalWrite(PCF_3, i & 0x08 ? HIGH : LOW);
    digitalWrite(PCF_4, i & 0x10 ? HIGH : LOW);
    digitalWrite(PCF_5, i & 0x20 ? HIGH : LOW);
    digitalWrite(PCF_6, i & 0x40 ? HIGH : LOW);
    digitalWrite(PCF_7, i & 0x80 ? HIGH : LOW);


    Wire.requestFrom(ACE_ADDR, 1);
    uint8_t result = Wire.read();

    if ( i != result) {
      error = 1;
      Serial.print(" error at ");
      Serial.print(i);
      Serial.print(" got ");
      Serial.println(result);
      digitalWrite(RED_LED, HIGH);
    }
  }
  if (error == 0) {
    digitalWrite(GRN_LED, HIGH);
  }
}

void loop() {

}
