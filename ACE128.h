/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013 Alastair Young.  All right reserved.
*/

// ensure this library description is only included once
#ifndef ACE28_h
#define ACE128_h

// include types & constants of Wiring core API
#include <Arduino.h>
#include <Wire.h>

// library interface description
class ACE128
{
  // user-accessible "public" interface
  public:
    // constructor takes relative i2caddr and pointer to PROGMEM map table
    // example: ACE128 myACE((uint8_t)0, (uint8_t*)encoderMap_12345678);
    // see make_encodermap example sketch for alternate pin mappings 
    ACE128(uint8_t i2caddr, uint8_t *map); 
    void begin();                  // initializes IO expander, call from setup()
    uint8_t upos();                // returns logical position 0 -> 127
    int8_t pos();                  // returns logical position -64 -> +63
    void setZero(uint8_t rawPos);  // sets logical zero position
    uint8_t getZero();             // returns logical zero position
    uint8_t rawPos();              // returns raw mechanical position
    uint8_t acePins();             // returns gray code inputs
    void reverse(boolean reverse); // set counter-clockwise operation
    // library-accessible "private" interface
  private:
    uint8_t _zero;                 // raw position of logical zero
    int8_t _reverse;              // counter-clockwise 
    int _i2caddr;                  // i2c bus address
    uint8_t *_map;                 // pointer to PROGMEM map table
};


// MCP23008 IO expander 
#define MCP23008_ADDRESS 0x20
#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A

#endif // ACE128_h

