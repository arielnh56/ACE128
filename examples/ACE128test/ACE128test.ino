/*  ACE128 Demonstrator
 *  
 *  This demo provides examples of all the options and functions available in this library.
 *  You should be able to find everything you need from here to use in your own sketch.
 *  This makes is very long, detailed and wordy.
 *  
 *  See the ace128_0x20 and ace128_0x38 examples for short specific tests for the manufactured modules.
 *  
 *  As distributed it can be used to test the functionality of the Absolute Encoder modules 
 *  that I make and distribute online on AVR platforms e.g. Arduino Unos.
 *  
 *  If you wire a button between pin 13 and ground, you can test setting logical zero
 *  
 *  For use on non-AVR chips (e.g. Arduino MKR) and use without PCF8574(A) pin expanders, see below.
 *  
 *  The inline comments do not assume the use of one of my modules, so there are references to
 *  mis-wiring and alternate wiring which are relevent to people building their own circuits or breadboarding.
 *  
 *  The value below is the address of your module. Set this to the correct value.
 *  For purchased modules this is the base address of the module.
 */

// PCF8574 0x20
// PCF8574A 0x38
// MCP23008 0x00 (see notes below) 
#define ACE_ADDR 0x20 

/*   
 *  this sketch displays the current position in all five forms on the serial monitor at 9600 baud
 *    pin  = the raw gray-code from the pins. As you rotate this will rise and fall in steps of 1,2,4,8,16,32,64 or 128
 *    raw  = the output from the encoder map table. This should rise from 0 - 127 as you rotate clockwise
 *           if it jumps around and shows 255 a lot - your wiring does not match your encoder map. Check your wiring
 *           and your object declaration. See the make_encodermap example sketch to create encoder maps for
 *           alternate pin arrangements
 *    pos  = this is the raw value converted to the range -64 - +63 relative to the logical zero. Logical zero is
 *           wherever it was when the sketch started, unless the value was saved in EEPROM or reset.
 *    upos = this is the raw value converted to the range 0 - 127 relative to the logical zero
 *    mpos = this is like pos, but goes multiturn -32768 - 32767
 *    
 *    For this sketch we need to select what displays we will use
 */
#define DISPLAY_SERIAL  // display on serial at 9600 baud
//#define DISPLAY_LCD     // display on LCD with PCF8574 backpack at 0x27


#ifdef DISPLAY_LCD
  #include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27);  // 1602 LCD on 0x27
#endif
 /*
 *  Your sketch needs to include the ACE128.h library near the top
 *  For the following non-standard options you need to edit some options in ACE128.h
 *     disabling EEPROM storage code on AVR platforms
 *     enabling I2C EEPROM storage on all platforms - you can also change the default I2C address of this EEPROM
 *     enabling direct-wire to the ACE128 - this disables all the pin-expander chip code
 *     enabling support for MCP23008 pin expanders (this does not disable PCF8574(A) support)
 *  These options will be documented in detail below.
 *  Note that if you are having trouble editing the ACE128.h file in the library folder you can copy it to your sketch folder
 *  and edit it there. If you do this, use the second #include format below
 */
#include <ACE128.h>  // Include the ACE128.h from the library folder
// #include "ACE128.h"  // Include the ACE128.h from the sketch folder

/*  Now we include the encoder map.
 *  This is a table of all the 256 possible pin combinations and how they map to the 128 gray code values. 
 *  Invalid locations are filled with 255. 
 *  The modules I make all use the 87654321 map and this is silkscreened on the circuit board. If you are building your
 *  own circuits or breadboarding you may need another encoder map. Several are available in the library, as well as an
 *  example sketch to generate new ones.
 *  Each map is identified by the sequence of numbers 1 thru 8 in the order that they appear on the P0 - P7 pins of the chip
 */
#include <ACE128map87654321.h> // mapping for pin order 87654321

/*  MCP23008 pin expanders are optionally supported. I built my first modules using these but the code is no longer
 *  included by default and must be enabled by defining ACE128_MCP23008 in ACE128.h 
 *  Both modules can be used on the same project but as the MCP and PCF address ranges overlap we tell them apart by 
 *  calling the contructor with I2C address range 0x00-0x07 which is mapped internally to the actual 0x20-0x27 range
 *  The following little bit of preprocessor code helps for this sketch as we need the mapped address
 *  for bus probing
 */
#ifdef ACE128_MCP23008
  #define ACE_PROBE_ADDR ACE_ADDR | 0x20
#else
  #define ACE_PROBE_ADDR ACE_ADDR
#endif
/*
 *  Now construct an ACE128 object for each unit in your project
 *  There are 4 constructors available. Providing a choice of I2C or direct pin mode and with or without EEPROM state storage
 */
ACE128 myACE(ACE_ADDR, (uint8_t*)encoderMap_87654321); // I2C without using EEPROM
// the EEPROM contructor is not available on SAM based platforms unless ACE128_I2C_EEPROM is defined in ACE128.h
//ACE128 myACE(ACE_ADDR, (uint8_t*)encoderMap_87654321, 0); // I2C using EEPROM address 0

/* 
 *  If wiring directly to the ACE128 sensor, the constructors below are enabled by defining ACE128_ARDUINO_PINS
 *  in ACE128.h - this also disable I2C chip capabilities.
 *  In this case the 8 numbers identify arduino digital pins. So you can shuffle those numbers around instead of 
 *  changing the encoder map. Have fun!
 */
//ACE128 myACE(uint8_t{2,3,4,5,6,7,8,9}, (uint8_t*)encoderMap_87654321); // direct pins without using EEPROM
// the EEPROM contructor is not available on SAM based platforms unless ACE128_I2C_EEPROM is defined in ACE128.h
//ACE128 myACE(uint8_t{2,3,4,5,6,7,8,9}, (uint8_t*)encoderMap_87654321, 0 ); // direct pins using EEPROM address 0

// the following stuff is for this example script
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
  // display initialization
#ifdef DISPLAY_SERIAL
  Serial.begin(9600);
#endif
#ifdef DISPLAY_LCD
  lcd.begin(16, 2); // 16 x 2 display common with Arduino kits
  lcd.setBacklight(127);
#endif
  // this bit of code probes the bus to see if it can talk to the chip
  // not required - just for debug and testing
  int error = 1;
  while (error != 0) {
#ifdef DISPLAY_SERIAL
    Serial.print("looking for ACE128 at 0x");
    Serial.println(ACE_PROBE_ADDR, HEX);
#endif
#ifdef DISPLAY_LCD
    lcd.setCursor(0, 0);
    lcd.print("looking for");
    lcd.setCursor(0, 1);
    lcd.print("ACE-128 at 0x");
    lcd.print(ACE_PROBE_ADDR, HEX);
#endif
    Wire.beginTransmission(ACE_PROBE_ADDR);
    error = Wire.endTransmission();
    delay(100);
  }
  // this is the meat - you need this bit
  myACE.begin();    // this is required for each instance, initializes the pins

  // some optional stuff - if you need it to run backwards
  // myACE.reverse(true);  // uncomment this for counter-clockwise operation

  // your sketch won't need this
  // this pulls and saves the current raw pin loctions
  // this is a prep for the physical pin check in loop()
  pinPos = myACE.acePins();          // get IO expander pins
  oldPos = pinPos;                 // remember where we are
#ifdef DISPLAY_SERIAL
    Serial.println("looking for pins - turn the ACE-128");
#endif
#ifdef DISPLAY_LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("looking for pins");
  lcd.setCursor(0, 1);
  lcd.print("turn the ACE-128");
#endif
  pinMode(ZERO, INPUT_PULLUP);    // configure set-zero button
}


void loop() {
  /*  About logical zero
   *  The sensor has an unmarked round shaft which you are probably going to screw a knob onto which will conceal even
   *  the slot on the end. The chance of getting this lined up with the sensors's actual zero is almost zero.
   *  So we set our knob to where we want zero to be and then we tell the software that that is where zero is. 
   *  If EEPROM storage is in use the physical location of the device will be stored to EEPROM when we setZero() or 
   *  setMpos(), and read back when we start up
   *  
   *  You can set zero in three ways
   *  
   *  myACE.setZero()  will define the current location as logical zero 
   *                   You want this one if you are using pos() or upos()
   *  myACE.setZero(N) when 0 <= N <= 127 will set logical zero to be the raw physical location specified by N 
   *                   You probably will not want this one. Ever.
   *  myAce.setMpos(N) when N is a 16 bit signed integer set the current multiturn position to N. Use this if you are using mpos().
   *                   It sets the multiturn counter to int(N/128) and adjusts the logical zero so that the current location
   *                   reads back N in mpos(). So if you want the current location to be zero, use setMpos(0). 
   *                   
   *  About multiturn
   *  Whenever the sensor is queried the code looks to see if it has "rolled over" up or down and keeps track of this in the
   *  multiturn counter. Whenever it is updated the value is stored in EEPROM (2 bytes). The limitation of this is that if,
   *  say, you turn the sensor clockwise a 3/4 turn between queries the software will see a 1/4 turn anti-clockwise. So poll
   *  frequently relative to twiddling speed. This should not be hard. Every loop() is recommended.
   */
  
  if (digitalRead(ZERO) == 0) {     // check set-zero button
    myACE.setMpos(0);               // set logical multiturn zero to current position
    oldPos = 255;                   // force display update
  }

  // these are the 5 ways you can pull the position of your encoder
  // your sketch probably only needs one of these to suit your application
  // do this every loop()
  pinPos = myACE.acePins();          // get IO expander pins - this is for debug
  rawPos = myACE.rawPos();           // get raw mechanical position - this for debug
  pos = myACE.pos();                 // get logical position - signed -64 to +63
  upos = myACE.upos();               // get logical position - unsigned 0 to +127
  mpos = myACE.mpos();               // get multiturn position - signed -32768 to +32767

  // And you're done. 
  // The rest of this is display output that I use for module testin
  if (pinPos != oldPos) {            // did we move?
    seen |= pinPos ^ oldPos;         // what changed?
    oldPos = pinPos;                 // remember where we are
    // first make sure all the pins are jumping up and down when we twiddle
    // this catches dry joints and shorts
    if (seen < 255) {
#ifdef DISPLAY_SERIAL
      Serial.print("looking for pins ");
#endif
#ifdef DISPLAY_LCD
   lcd.clear();
   lcd.print("looking for pins");
   lcd.setCursor(0, 1);
#endif
      for (uint8_t i = 0; i <= 7; i++) {
        if (! (seen & 1 << i)) {
#ifdef DISPLAY_SERIAL
          Serial.print(i, DEC);
#endif
#ifdef DISPLAY_LCD
          lcd.print(i, DEC);
#endif
        }
      }
#ifdef DISPLAY_SERIAL
      Serial.println("");
#endif
    // then we display stuff forever
    } else {
#ifdef DISPLAY_SERIAL
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
#endif
#ifdef DISPLAY_LCD
      lcd.clear();
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
#endif
    }
  }
}
