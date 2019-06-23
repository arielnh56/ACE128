/*  ACE128 Basic Test 0x38
 *  
 *  This is a basic validation test for the Absolute Encoder Module with 
 *  base address 0x38. This should work out of the box with your module.
 *  
 *  It does not demonstrate all the options and variations.
 *  
 *  See ACE128test example for all the options or if something goes wrong
 *  
 *  this sketch displays the current position in the three useful forms on the serial monitor at 9600 baud
 *    pos  = this is the raw value converted to the range -64 - +63 relative to the logical zero. Logical zero is
 *           wherever it was when the sketch started, unless the value was saved in EEPROM or reset.
 *    upos = this is the raw value converted to the range 0 - 127 relative to the logical zero
 *    mpos = this is like pos, but goes multiturn -32768 - 32767
 *  
 *  The value below is the address of your module. Set this to the correct value.
 *  For purchased modules this is the base address of the module.
 */

// PCF8574A 0x38
#define ACE_ADDR 0x38 

#include <ACE128.h>  // Include the ACE128.h from the library folder
#include <ACE128map87654321.h> // mapping for pin order 87654321

ACE128 myACE(ACE_ADDR, (uint8_t*)encoderMap_87654321); // I2C without using EEPROM
uint8_t oldPos = 255;
uint8_t upos;
int8_t pos;
int16_t mpos;

void setup() {
  Serial.begin(9600);
  myACE.begin();    // this is required for each instance, initializes the pins
}


void loop() {
  pos = myACE.pos();                 // get logical position - signed -64 to +63
  upos = myACE.upos();               // get logical position - unsigned 0 to +127
  mpos = myACE.mpos();               // get multiturn position - signed -32768 to +32767

  if (upos != oldPos) {            // did we move?
    oldPos = upos;                 // remember where we are
    Serial.print(" pos ");
    Serial.print(pos, DEC);
    Serial.print(" upos ");
    Serial.print(upos, DEC);
    Serial.print(" mpos ");
    Serial.println(mpos, DEC);
  }
}
