/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013,2015,2017,2018 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/

#include <Arduino.h>

// include description files for other libraries used
#include <Wire.h>
#include <ACE128.h>

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

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
  else                                       // use zero-indexed address to identify MCP23008 - backwards compatible
  {
    _i2caddr = (i2caddr & 0x7) | ACE128_MCP23008_ADDRESS;   // map lower bits to MCP23008
    _chip = ACE128_MCP23008_ADDRESS;                        // remember what chip
  }
  _reverse = false;                        // clockwise
  _zero = 0;                               // set zero position
  _map = map;                              // mapping table in PROGMEM
#ifndef ACE128_EEPROM_NONE
  _eeAddr = eeAddr;                       // multiturn save location
#endif
}

#ifdef ACE128_EEPROM_NONE
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map)
#else
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map) : ACE128::ACE128(pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7, map, -1) {}
ACE128::ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map, int16_t eeAddr)
#endif
{
  // initialize this instance's variables
  _chip = ACE128_ARDUINO_PINS;
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

// Initializer /////////////////////////////////////////////////////////////////
// Call this fuction during setup to initialize MCP23008

void ACE128::begin()
{
#ifndef ACE128_EEPROM_I2C    // we need only one Write.begin()
  if (_chip != ACE128_ARDUINO_PINS)
#endif
    // initialize the chip
    Wire.begin();        // join i2c bus (address optional for master)

  if (_chip == ACE128_ARDUINO_PINS)
  {
    for (uint8_t i = 0; i <= 7; i++) {
      pinMode(_pins[i], INPUT_PULLUP);
    }
  }
  else
  {
    Wire.beginTransmission(_i2caddr);
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
    {
      Wire.write((uint8_t)0xFF);  // set all pins up. pulldown for input
    }
    Wire.endTransmission();
  }
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
  if (_chip == ACE128_ARDUINO_PINS)
  {
    uint8_t pinbits = 0;
    for (uint8_t pin = 0; pin <= 7; pin++) {
      pinbits |= (uint8_t)digitalRead(_pins[pin]) << pin;
    }
    return(pinbits);
  }
  else
  {
    // read one byte from the GPIO
    if (_chip == ACE128_MCP23008_ADDRESS)
    {
      Wire.beginTransmission(_i2caddr);
      Wire.write((uint8_t)ACE128_MCP23008_GPIO);
      Wire.endTransmission();
    }
    Wire.requestFrom(_i2caddr, 1);
    return (Wire.read());
  }
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
// this also sets zero, so if you are saving zero, call setZero() afterwards.
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
