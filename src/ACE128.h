#ifndef ACE128_h
#define ACE128_h
/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013-2019 Alastair Young.
  This project is licensed under the terms of the MIT license.

  This contains all of the source code. The main code has been moved from the cpp file to the .h
  file to allow functioning end-user preprocessor macros within the Arduino IDE environment.
  This means it all gets compiled every time, but it makes it easier on the end user and lets me
  slice out all unnecessary code for the user's program, limiting the final footprint - which matters
  on the little AVRs.
*/

// Use these preprocessor #define statements to configure you project. 
// Uncomment them here to set them for all projects
// Unfortuately due to the wierdness of the Arduino IDE, #defines in your sketch don't propagate here

// Set up the EEPROM storage option. 
// Include one of these to override default behavior
// #define ACE128_EEPROM_AVR  // Internal EEPROM.h This is the default for AVR e.g. Arduino Uno, AtTiny ATmega ATtiny etc
// #define ACE128_EEPROM_NONE // No EEPROM storage - default for non-AVR e.g. MKR series, SAM
// #define ACE128_EEPROM_I2C  // I2C EEPROM - e.g.  Microchip 24CW160T
// #define ACE128_EEPROM_ADDR 0x50  // address for the I2C chip. If you leave this undefined it defaults to 0x50

// Enable MCP23008 support. 
// Before v2.0.0 this was available by default. Now you need to set this flag.
// #define ACE128_MCP23008 // Enable MCP23008 support

// Use direct pin connection, disable pin expander code
// Also disables all I2C activity unless ACE128_EEPROM_I2C is defined
// Prior to v2.0.0 this was available by default along with the pin expanders
// #define ACE128_ARDUINO_PINS

// end of user configurable #define statements

// ensure mutual exclusion and defaults
#if defined(ACE128_EEPROM_AVR)
  #undef ACE128_EEPROM_NONE
  #undef ACE128_EEPROM_I2C
#elif defined(ACE128_EEPROM_NONE)
  #undef ACE128_EEPROM_AVR
  #undef ACE128_EEPROM_I2C
#elif defined(ACE128_EEPROM_I2C)
  #undef ACE128_EEPROM_AVR
  #undef ACE128_EEPROM_NONE
#elif defined(ARDUINO_ARCH_AVR)
  #define ACE128_EEPROM_AVR
#else
  #define ACE128_EEPROM_NONE
#endif

#if defined(ACE128_EEPROM_I2C) || !defined(ACE128_ARDUINO_PINS)
  #define ACE128_I2C
#endif

// include types & constants of Wiring core API
#include <Arduino.h>
#ifdef ACE128_I2C
  #include <Wire.h>
#endif

// Select EEPROM stuff, if any
#if defined(ACE128_EEPROM_AVR)
  #include <EEPROM.h>
#elif defined(ACE128_EEPROM_I2C)
  #if !defined(ACE128_EEPROM_ADDR)
    #define ACE128_EEPROM_ADDR 0x50   // user changeable
  #endif
#endif 
// we store the encoder maps in program space
#include <avr/pgmspace.h>

// library interface description
class ACE128
{
    // user-accessible "public" interface
  public:
    // I2C constructor takes i2caddr and pointer to PROGMEM map table
    // example: ACE128 myACE((uint8_t)0, (uint8_t*)encoderMap_12345678);
    // see make_encodermap example sketch for alternate pin mappings
    // Select with the following addresses
    // 0x00 - 0x07 MCP23008 addresses 0x20-0x27. Requires ACE128_MCP23008 to be #defined
    // 0x20 - 0x27 PCF8574
    // 0x38 - 0x3F PCF8574A
    // final optional eeAddr parameter sets and enables EEPROM state save for logical zero and multiturn position
    // direct pin constructors are similar but instead of the I2C address, you list the 8 arduino pins used
#ifdef ACE128_ARDUINO_PINS
  #ifndef ACE128_EEPROM_NONE
    ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map, int16_t eeAddr);
  #endif
    ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map);
#else
  #ifndef ACE128_EEPROM_NONE
    ACE128(uint8_t i2caddr, uint8_t *map, int16_t eeAddr);
  #endif
    ACE128(uint8_t i2caddr, uint8_t *map);
#endif
    void begin();                  // initializes IO expander, call from setup()
    uint8_t upos();                // returns logical position 0 -> 127
    int8_t pos();                  // returns logical position -64 -> +63
    int16_t mpos();                // returns multiturn position -32768 -> +32767
    void setMpos(int16_t mPos);    // sets current position to multiturn value - also changes zero
    void setZero();                // sets logical zero to current position
    void setZero(uint8_t rawPos);  // sets logical zero position
    uint8_t getZero();             // returns logical zero position
    uint8_t rawPos();              // returns raw mechanical position
    uint8_t acePins();             // returns gray code inputs
    void reverse(boolean reverse); // set counter-clockwise operation
    // library-accessible "private" interface
  private:
    uint8_t _zero;                 // raw position of logical zero
    int8_t _reverse;               // counter-clockwise
    uint8_t *_map;                 // pointer to PROGMEM map table
    int16_t _mpos;                 // multiturn offset
    int8_t _lastpos;               // last upos
#ifndef ACE128_EEPROM_NONE
    int16_t _eeAddr;               // multiturn save location (2 bytes)
    void _eeprom_read_settings(); // read _mpos and _zero from 
    void _eeprom_write_mpos();    // write _mpos to eeprom
    void _eeprom_write_zero();    // write _zero to eeprom 
#endif
    int8_t _raw2pos(int8_t pos);   // convert rawPos() value to pos()
#ifdef ACE128_ARDUINO_PINS
    uint8_t _pins[8];              // store pins for direct attach mode
#else
    uint8_t _chip;                 // chip type - derived from i2c address
    int _i2caddr;                  // i2c bus address
#endif
};


#ifdef ACE128_MCP23008
// MCP23008 IO expander
  #define ACE128_MCP23008_ADDRESS 0x20
  #define ACE128_MCP23008_IODIR   0x00
  #define ACE128_MCP23008_IPOL    0x01
  #define ACE128_MCP23008_GPINTEN 0x02
  #define ACE128_MCP23008_DEFVAL  0x03
  #define ACE128_MCP23008_INTCON  0x04
  #define ACE128_MCP23008_IOCON   0x05
  #define ACE128_MCP23008_GPPU    0x06
  #define ACE128_MCP23008_INTF    0x07
  #define ACE128_MCP23008_INTCAP  0x08
  #define ACE128_MCP23008_GPIO    0x09
  #define ACE128_MCP23008_OLAT    0x0A
#endif

// PCF8574 family
#define ACE128_PCF8574_ADDRESS  0x20
#define ACE128_PCF8574A_ADDRESS 0x38


// former cpp code starts here

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

#ifdef ACE128_ARDUINO_PINS
  #ifdef ACE128_EEPROM_NONE
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map)
  #else
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map) : ACE128::ACE128(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, map, -1) {}
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map, int16_t eeAddr)
  #endif
{
  // initialize this instance's variables
  _pins[0] = pin0;
  _pins[1] = pin1;
  _pins[2] = pin2;
  _pins[3] = pin3;
  _pins[4] = pin4;
  _pins[5] = pin5;
  _pins[6] = pin6;
  _pins[7] = pin7;
  _reverse = false;                        // clockwise
  _zero = 0;                               // set zero position
  _map = map;                              // mapping table in PROGMEM
  #ifndef ACE128_EEPROM_NONE
  _eeAddr = eeAddr;                       // multiturn save location
  #endif
}
#else // !ACE128_ARDUINO_PINS
  #ifdef ACE128_EEPROM_NONE
ACE128::ACE128(uint8_t i2caddr, uint8_t *map)
  #else
ACE128::ACE128(uint8_t i2caddr, uint8_t *map) : ACE128::ACE128(i2caddr, map, -1) {}
ACE128::ACE128(uint8_t i2caddr, uint8_t *map, int16_t eeAddr)
  #endif
{
  // initialize this instance's variables
  if ((i2caddr & 0x78) == ACE128_PCF8574_ADDRESS || (i2caddr & 0x78) == ACE128_PCF8574A_ADDRESS)
  {
    _i2caddr = i2caddr;                      // save address
    _chip = ACE128_PCF8574A_ADDRESS;         // PCF8574 shares address space with MCP23008
  }
  #if defined(ACE128_MCP23008)
  else                                       // use zero-indexed address to identify MCP23008 - backwards compatible
  {
    _i2caddr = (i2caddr & 0x7) | ACE128_MCP23008_ADDRESS;   // map lower bits to MCP23008
    _chip = ACE128_MCP23008_ADDRESS;                        // remember what chip
  }
  #endif
  _reverse = false;                        // clockwise
  _zero = 0;                               // set zero position
  _map = map;                              // mapping table in PROGMEM
  #ifndef ACE128_EEPROM_NONE
  _eeAddr = eeAddr;                       // multiturn save location
  #endif
}
#endif // ACE128_ARDUINO_PINS

// Initializer /////////////////////////////////////////////////////////////////
// Call this fuction during setup to initialize the chip

void ACE128::begin()
{
#ifdef ACE128_I2C    // if we are using I2C, initialize it
  Wire.begin();      // join i2c bus (address optional for master)
#endif

#ifdef ACE128_ARDUINO_PINS
  // initialize the pins
  for (uint8_t i = 0; i <= 7; i++) {
    pinMode(_pins[i], INPUT_PULLUP);
  }
#else
  Wire.beginTransmission(_i2caddr);
  #ifdef ACE128_MCP23008
  if (_chip == ACE128_MCP23008_ADDRESS)
  {
    Wire.write((uint8_t)ACE128_MCP23008_IODIR); // MCP23008 lets us blast all registers
    Wire.write((uint8_t)0xFF);  // IODIR all inputs
    Wire.write((uint8_t)0x00);  // IPOL  do not invert
    Wire.write((uint8_t)0x00);  // GPINTEN disable interrupt
    Wire.write((uint8_t)0x00);  // DEFVAL disabled
    Wire.write((uint8_t)0x00);  // INTCON disabled
    Wire.write((uint8_t)0x00);  // IOCON no special config
    Wire.write((uint8_t)0xFF);  // GPPU pullup all inputs
    Wire.write((uint8_t)0x00);  // INTF disabled
    Wire.write((uint8_t)0x00);  // INTCAP disabled
    Wire.write((uint8_t)0x00);  // GPIO
    Wire.write((uint8_t)0x00);  // OLAT
  }
  else if (_chip == ACE128_PCF8574A_ADDRESS)
  #endif // ACE128_MCP23008
  {
    Wire.write((uint8_t)0xFF);  // set all pins up. pulldown for input
  }
  Wire.endTransmission();
#endif
#ifndef ACE128_EEPROM_NONE
  if (_eeAddr >= 0)
  {
    _eeprom_read_settings();
    _lastpos = pos();
  }
  else
#endif // ACE128_EEPROM_NONE
  {
    _mpos = 0;
    _zero = rawPos(); // set zero to where we happen to be
    _lastpos = 0;
  }
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

// returns the current value on the IO expander pins
// Used internally, but exposed to help verify mapping tables
// If you ever get a 255 from a mapping table, something is wrong
uint8_t ACE128::acePins(void)
{
#ifdef ACE128_ARDUINO_PINS
  uint8_t pinbits = 0;
  for (uint8_t pin = 0; pin <= 7; pin++) {
    pinbits |= (uint8_t)digitalRead(_pins[pin]) << pin;
  }
  return(pinbits);
#else
  // read one byte from the chip
  #if defined(ACE128_MCP23008)
  if (_chip == ACE128_MCP23008_ADDRESS)
  {
    Wire.beginTransmission(_i2caddr);
    Wire.write((uint8_t)ACE128_MCP23008_GPIO);
    Wire.endTransmission();
  }
  #endif
  Wire.requestFrom(_i2caddr, 1);
  return (Wire.read());
#endif
}

// returns current raw position
uint8_t ACE128::rawPos(void)
{
  // look up our raw position in the mapping table
  return (pgm_read_byte(_map + acePins()));
}

// returns unsigned position 0 - 127
uint8_t ACE128::upos(void)
{
  uint8_t pos = rawPos();   // get raw position
  pos -= _zero;             // adjust for logical zero
  if (_reverse) pos *= -1;  // reverse direction

  pos &= 0x7F; // clear the 8bit neg bit
  return (pos);
}

// returns signed position -64 - +63
int8_t ACE128::pos(void)
{
  return (_raw2pos(rawPos()));
}

int8_t ACE128::_raw2pos(int8_t pos) {
  pos -= _zero;   // adjust for logical zero
  if (_reverse) pos *= -1;    // reverse direction
  // 7bit signed numbers need to copy their neg bit to the 8bit position
  if ( pos & 0x40 ) { // check for 7bit neg bit
    pos |= 0x80; // set 8bit neg bit
  } else {
    pos &= 0x7F; // clear 8bit neg bit
  }
  return (pos);
}

int16_t ACE128::mpos(void)
{
  int16_t currentpos = pos();
  if ((int16_t)_lastpos - currentpos > 0x40)    // more than half a turn smaller - we rolled up
  {
    _mpos += 0x80;
  }
  else if (currentpos - (int16_t)_lastpos > 0x40)   // more than half a turn bigger - we rolled down
  {
    _mpos -= 0x80;
  }
#ifndef ACE128_EEPROM_NONE
  if (_eeAddr >= 0)
  {
    _eeprom_write_mpos();
  }
#endif
  _lastpos = currentpos;
  return _mpos + currentpos;
}

// sets logical zero position
void ACE128::setZero(uint8_t rawPos)
{
  _zero = rawPos & 0x7f;  // mask to 7bit
#ifndef ACE128_EEPROM_NONE
  if (_eeAddr >= 0)
  {
    _eeprom_write_zero();
  }
#endif
}

// set current position to zero
void ACE128::setZero()
{
  setZero(rawPos());
}

// returns current logical zero
uint8_t ACE128::getZero(void)
{
  return (_zero);
}

// set multiturn position
void ACE128::setMpos(int16_t mPos)
{
  uint8_t rawpos = rawPos();
  setZero(rawpos - (uint8_t)(mPos & 0x7f));  // mask to 7bit
  _lastpos = _raw2pos(rawpos);
  _mpos = (mPos - _lastpos) & 0xFF80;          // mask higher 9 bits
#ifndef ACE128_EEPROM_NONE
  if (_eeAddr >= 0)
  {
    _eeprom_write_mpos();
  }
#endif
}


// set reverse if true - i.e. counter-clockwise
void ACE128::reverse(boolean reverse)
{
  _reverse = reverse;
}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library
#ifndef ACE128_EEPROM_NONE
// read _mpos and _zero from
void ACE128::_eeprom_read_settings()
{
  #if defined(ACE128_EEPROM_I2C)
  Wire.beginTransmission(ACE128_EEPROM_ADDR);
  Wire.write((uint8_t) (_eeAddr >> 8));
  Wire.write((uint8_t) _eeAddr );
  Wire.endTransmission(false);
  Wire.requestFrom(ACE128_EEPROM_ADDR, 3);
  _mpos = (Wire.read() + (Wire.read() << 8));
  _zero = (Wire.read());
  #elif defined(ACE128_EEPROM_AVR)
  EEPROM.get(_eeAddr, _mpos);
  EEPROM.get(_eeAddr + sizeof(_mpos), _zero);
  #endif
}

// I2C EEPROM write functions are very simple to suit this application
// Note no update function. I2C EEPROM has 10X endurance of Atmega EEPROM
// which in turn has 2X endurance of ACE-128 sensor. Not worth the space it takes to code...

// write _mpos to eeprom
void ACE128::_eeprom_write_mpos()
{
  #if defined(ACE128_EEPROM_I2C)
  Wire.beginTransmission(ACE128_EEPROM_ADDR);
  Wire.write((uint8_t) (_eeAddr >> 8));
  Wire.write((uint8_t) _eeAddr );
  Wire.write((uint8_t) _mpos );
  Wire.write((uint8_t) (_mpos >> 8));
  Wire.endTransmission();
  #elif defined(ACE128_EEPROM_AVR)
  EEPROM.put(_eeAddr, _mpos);
  #endif
}

// write _zero to eeprom
void ACE128::_eeprom_write_zero()
{
  uint16_t eeAddr = eeAddr + sizeof(_mpos);
  #if defined(ACE128_EEPROM_I2C)
  Wire.beginTransmission(ACE128_EEPROM_ADDR);
  Wire.write((uint8_t) (eeAddr >> 8));
  Wire.write((uint8_t) eeAddr );
  Wire.write((uint8_t) _zero );
  Wire.endTransmission();
  #elif defined(ACE128_EEPROM_AVR)
  EEPROM.update(eeAddr, _zero);
  #endif
}

#endif // ACE128_EEPROM_NONE

#endif // ACE128_h
