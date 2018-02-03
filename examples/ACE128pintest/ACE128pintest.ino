/* ACE PIN Serial Tester
   this displays the current position in all five forms on the serial monitor
     pin = the raw gray-code from the pins. As you rotate this will rise and fall in steps
           of 1,2,4,8,16,32,64 or 128
     raw = the output from the encoder map table. This should rise from 0 - 127 as you rotate clockwise
           if it jumps around and shows 255 a lot - your wiring does not match your encode map. Check your wiring
           and your object declaration. See the make_encodermap example sketch to create encoder maps for
           alternate pin arrangements
     pos = this is the raw value converted to the range -64 - +63 relative to the logical zero. Logical zero is
           wherever it was when the sketch started, or wherever it was when pin 13 was last grounded.
     upos = this is the raw value converted to the range 0 - 127 relative to the logical zero
     mpos = this is like pos, but goes multiturn -32768 - 32767
*/


// Include the encoder library and maps
#include <ACE128.h>  // Absolute Contact Encoder
#include <ACE128map12345678.h> // mapping for pin order 12345678
//#include <ACE128map87654321.h> // mapping for pin order 87654321
#include <Wire.h> // I2C bus communication library - required to support ACE128


// Create an ACE128 instance called myACE
ACE128 myACE(2,3,4,5,6,7,8,9, (uint8_t*)encoderMap_12345678);
//ACE128 myACE(uint8_t{7,6,5,4,3,2,1,0}, (uint8_t*)encoderMap_87654321);

// set-zero button on pin 13
// - button to ground e.g. MakerShield button
const int ZERO = 13;
uint8_t pinPos = 0; // pin values
uint8_t rawPos = 0;
uint8_t upos = 0;
uint8_t oldPos = 255;
int8_t pos;
int16_t mpos;
uint8_t seen = 0;

void setup() {
  int error = 1;
  myACE.begin();    // this is required for each instance, initializes the pins
  // myACE.reverse(true);  // uncomment this for counter-clockwise operation
  pinMode(ZERO, INPUT_PULLUP);    // configure set-zero button
  pinPos = myACE.acePins();          // get IO expander pins
  oldPos = pinPos;                 // remember where we are
  Serial.begin(9600);
  Serial.println(myACE.acePins());
}


void loop() {
  if (digitalRead(ZERO) == 0) {      // check set-zero button

    // myACE.setMpos(6723);  // set current position to 6723
    myACE.setMpos(0);    // set logical multiturn zero to current position
    // myACE.setZero();     // set logical zero to current position
    // myACE.reverse(true);  // set reverse
    oldPos = 255;                   // force display update
  }
  pinPos = myACE.acePins();          // get IO expander pins
  rawPos = myACE.rawPos();           // get raw mechanical position
  pos = myACE.pos();                 // get logical position - signed
  upos = myACE.upos();               // get logical position - unsigned
  mpos = myACE.mpos();               // get multiturn position - signed

  if (pinPos != oldPos) {            // did we move?
    seen |= pinPos ^ oldPos;         // what changed?
    oldPos = pinPos;                 // remember where we are
    if (seen < 255) {
      Serial.println("looking for pins");
      for (uint8_t i = 0; i <= 7; i++) {
        if (! (seen & 1 << i)) {
          Serial.print(i, DEC);
        }
      }
      Serial.println("");
    } else {
      Serial.print("pin ");
      Serial.print(pinPos);
      Serial.print(" raw ");
      Serial.print(rawPos);
      Serial.print(" pos ");
      Serial.print(pos, DEC);
      Serial.print(" upos ");
      Serial.print(upos, DEC);
      Serial.print(" mpos ");
      Serial.println(mpos, DEC);
    }
  }
}


