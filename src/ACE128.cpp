/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013,2015,2017 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/

#include <Arduino.h>

// include description files for other libraries used 
#include <Wire.h>
#include <ACE128.h>
#include <EEPROM.h>
// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

ACE128::ACE128(uint8_t i2caddr, uint8_t *map) : ACE128::ACE128(i2caddr, map, -1) {}

ACE128::ACE128(uint8_t i2caddr, uint8_t *map, int16_t eeAddr)
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
  _eeAddr = eeAddr;                       // multiturn save location
}

// Initializer /////////////////////////////////////////////////////////////////
// Call this fuction during setup to initialize MCP23008
 
void ACE128::begin()
{
  // initialize the chip
  Wire.begin();        // join i2c bus (address optional for master)
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

  if (_eeAddr >= 0)
  {
    EEPROM.get(_eeAddr, _mpos);     
    EEPROM.get(_eeAddr + sizeof(_mpos), _zero);
    _lastpos = pos();
  }
  else
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
  // read one byte from the GPIO
  if (_chip == ACE128_MCP23008_ADDRESS)
  {
    Wire.beginTransmission(_i2caddr);
    Wire.write((uint8_t)ACE128_MCP23008_GPIO);
    Wire.endTransmission();
  }
  Wire.requestFrom(_i2caddr, 1);
  return(Wire.read());
}

// returns current raw position
uint8_t ACE128::rawPos(void)
{
  // look up our raw position in the mapping table
  return(pgm_read_byte(_map + acePins()));
}

// returns unsigned position 0 - 127
uint8_t ACE128::upos(void)
{
  uint8_t pos = rawPos();   // get raw position
  pos -= _zero;             // adjust for logical zero
  if (_reverse) pos *= -1;  // reverse direction
  
  pos &= 0x7F; // clear the 8bit neg bit
  return(pos);
}

// returns signed position -64 - +63
int8_t ACE128::pos(void)
{
  return(_raw2pos(rawPos()));
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
  return(pos);
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
  if (_eeAddr >= 0)
  {
    EEPROM.put(_eeAddr, _mpos);     
  }
  _lastpos = currentpos;
  return _mpos + currentpos;
}

// sets logical zero position
void ACE128::setZero(uint8_t rawPos)
{
  _zero = rawPos & 0x7f;  // mask to 7bit
  if (_eeAddr >= 0)
  {
    EEPROM.update(_eeAddr + sizeof(_mpos), _zero);
  }
}

// set current position to zero 
void ACE128::setZero()
{
  setZero(rawPos());
}

// returns current logical zero
uint8_t ACE128::getZero(void)
{
  return(_zero);
}

// set multiturn position
// this also sets zero, so if you are saving zero, call setZero() afterwards.
void ACE128::setMpos(int16_t mPos)
{
   uint8_t rawpos = rawPos();
   setZero(rawpos - (uint8_t)(mPos & 0x7f));  // mask to 7bit
  _lastpos = _raw2pos(rawpos);
  _mpos = (mPos - _lastpos) & 0xFF80;          // mask higher 9 bits
  if (_eeAddr >= 0)
  {
    EEPROM.put(_eeAddr, _mpos);     
  }
}


// set reverse if true - i.e. counter-clockwise
void ACE128::reverse(boolean reverse)
{
  _reverse = reverse;
}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library
// NONE
