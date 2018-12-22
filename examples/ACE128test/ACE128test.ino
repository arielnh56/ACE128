/* ACE LCD Tester
   this first checks that it can talk to the unit. check your addresses an wiring
   then you turn the knob and it looks for activity on all 8 pins
   if this phase does not complete, the display shows which pins are not showing activity
   check for wiring errors or dry joints
   then it displays the current position in all five forms on LCD display
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


#define LCD_I2C      // use LCD on PCF8574 address 0x27
#define LCD_I2C_ADDR 0x27
// Include the encoder library and maps
#include <ACE128.h>  // Absolute Contact Encoder
// #include <ACE128map12345678.h> // mapping for pin order 12345678
#include <ACE128map87654321.h> // mapping for pin order 87654321 (backpack module)
#include <Wire.h> // I2C bus communication library - required to support ACE128

#ifdef LCD_I2C
#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDR);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#else // use direct wire liquidcrystal library
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // your pins may vary
#endif

#define ACE_ADDR 0x38
// address range 0x00 - 0x07 -> MCP23008 at address 0x20 -> 0x27
// address range 0x20 - 0x27 -> PCF8574  at address 0x20 -> 0x27
// address range 0x38 - 0x3F -> PCF8574A at address 0x38 -> 0x3F
// note that the PCF8574 datasheet lists these addresses twice these values
// they count the read/write bit in the address, which is a source of confusion
// Create an ACE128 instance called myACE at address 0 (really 0x20) using the basic encoder map
// ACE128 myACE((uint8_t)0x00, (uint8_t*)encoderMap_12345678, -1); // datasheet basics - MCP23008
ACE128 myACE((uint8_t)ACE_ADDR, (uint8_t*)encoderMap_87654321); // backpack module - no save
// ACE128 myACE((uint8_t)ACE_ADDR, (uint8_t*)encoderMap_87654321, 0); // backpack module - save position 0

// set-zero button on pin 5
// - button to ground e.g. MakerShield button
const int ZERO = 5;
uint8_t pinPos = 0; // pin values from MCP23008 or PCF8574
uint8_t rawPos = 0;
uint8_t upos = 0;
uint8_t oldPos = 255;
int8_t pos;
int16_t mpos;
uint8_t seen = 0;

void setup() {
  int error = 1;
  lcd.begin(16, 2); // 16 x 2 display common with Arduino kits
#ifdef LCD_I2C
  lcd.setBacklight(127);
#endif
  // check that we can talk to it
  while (error != 0) {
    lcd.setCursor(0, 0);
    lcd.print("looking for");
    lcd.setCursor(0, 1);
    lcd.print("ACE-128");
    Wire.beginTransmission(ACE_ADDR);
    error = Wire.endTransmission();
  }
  myACE.begin();    // this is required for each instance, initializes the IO expander
  // myACE.reverse(true);  // uncomment this for counter-clockwise operation
  pinMode(ZERO, INPUT_PULLUP);    // configure set-zero button
  pinPos = myACE.acePins();          // get IO expander pins
  oldPos = pinPos;                 // remember where we are
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("looking for pins");
  lcd.setCursor(0, 1);
  lcd.print("turn the ACE-128");
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
    lcd.clear();             // all the rest is display stuff
    if (seen < 255) {
      lcd.setCursor(0, 0);
      lcd.print("looking for pins");
      lcd.setCursor(0, 1);
      for (uint8_t i = 0; i <= 7; i++) {
        if (! (seen & 1 << i)) {
          lcd.print(i, DEC);
        }
      }
    } else {
      lcd.setCursor(0, 0);
      lcd.print("pin ");
      lcd.print(pinPos);
      lcd.setCursor(8, 0);
      lcd.print("raw ");
      lcd.print(rawPos);
      lcd.setCursor(0, 1);
      lcd.print(pos, DEC);
      lcd.setCursor(4, 1);
      lcd.print(upos, DEC);
      lcd.setCursor(9, 1);
      lcd.print(mpos, DEC);
    }
  }
}
