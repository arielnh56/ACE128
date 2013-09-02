/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013 Alastair Young.  All right reserved.
*/

#include <Arduino.h>;

// include description files for other libraries used 
#include <Wire.h>
#include <pgmspace.h>
#include <ACE128.h>

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

ACE128::ACE128(uint8_t i2caddr, uint8_t *map)
{
  // initialize this instance's variables
  i2caddr &= 0x7;                          // 3 bits of address
  _i2caddr = i2caddr | MCP23008_ADDRESS;   // map to MCP23008
  _reverse = false;                        // clockwise
  _zero = 0;                               // set zero position
  _map = map;                              // mapping table in PROGMEM
}

// Initializer /////////////////////////////////////////////////////////////////
// Call this fuction during setup to initialize MCP23008
 
void ACE128::begin()
{
  // initialize the MCP23008
  Wire.begin();        // join i2c bus (address optional for master)
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)MCP23008_IODIR); // MCP23008 lets us blast all registers
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
  Wire.endTransmission();

  _zero = rawPos(); // set zero to where we happen to be
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

// returns the current value on the IO expander pins
// Used internally, but exposed to help verify mapping tables
// If you ever get a 255 from a mapping table, something is wrong
uint8_t ACE128::acePins(void)
{
  // read one byte from the GPIO
  Wire.beginTransmission(_i2caddr);
  Wire.write((uint8_t)MCP23008_GPIO);
  Wire.endTransmission();
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
  uint8_t pos;  
  pos = rawPos(); // read encoder and convert
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

// sets logical zero position
// use myACE.setZero(myACE.rawPos()) to set current position to zero 
void ACE128::setZero(uint8_t rawPos)
{
  _zero = rawPos & 0x7f;  // mask to 7bit
}

// returns current logical zero
uint8_t ACE128::getZero(void)
{
  return(_zero);
}

// set reverse if true - i.e. counter-clockwise
void ACE128::reverse(boolean reverse)
{
  _reverse = reverse;
}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library
// NONE
