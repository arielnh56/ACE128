// ACE LCD Tester
// Displays the current position in all four forms on LCD display

// Include the encoder library and maps
#include <ACE128.h>  // Absolute Contact Encoder
#include <ACE128map12345678.h> // mapping for pin order 12345678
// #include <ACE128map12348765.h> // mapping for pin order 12348765 (module)
#include <Wire.h> // I2C bus communication library - required to support ACE128

// LiquidCrystal used by this program for display
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // your pins may vary

// Create an ACE128 instance at address 0 (really 0x20) using the basic encoder map 
// see the make_encodermap example sketch to create maps for alternate pin mappings
ACE128 myACE((uint8_t)0, (uint8_t*)encoderMap_12345678); // datasheet basics
// ACE128 myACE((uint8_t)0, (uint8_t*)encoderMap_12348765); // module

// set-zero button on pin 13
// pullup resistor - button to ground e.g. MakerShield button
const int ZERO=13;

void setup() {
  lcd.begin(16, 2); // 16 x 2 display common with Arduino kits
  myACE.begin();    // this is required for each instance, initializes the IO expander
  // myACE.reverse(true);  // uncomment this for counter-clockwise operation
  pinMode(ZERO,INPUT);     // configure set-zero button
}

uint8_t pinPos=0; // pin values from MCP23008
uint8_t rawPos=0;
uint8_t upos=0;
uint8_t oldPos=255;
int8_t pos;

void loop() {
  delay(10);
  if (digitalRead(ZERO) == 0) {      // check set-zero button
     myACE.setZero(myACE.rawPos());  // set logical zero to current position
     oldPos=255;                     // force display update
  }
  pinPos=myACE.acePins();            // get IO expander pins
  rawPos=myACE.rawPos();             // get raw mechanical position
  pos=myACE.pos();                   // get logical position - signed
  upos=myACE.upos();                 // get logical position - unsigned
  if (pinPos != oldPos) {            // did we move?
    oldPos=pinPos;                   // remember where we are
    lcd.setCursor(0, 0);             // all the rest is display stuff
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("pin ");
    lcd.print(pinPos);
    lcd.setCursor(8, 0);
    lcd.print("pos  ");
    lcd.print(pos,DEC);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("raw ");
    lcd.print(rawPos);
    lcd.setCursor(8, 1);
    lcd.print("upos ");
    lcd.print(upos,DEC);
  }
}


