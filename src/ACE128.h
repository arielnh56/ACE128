/*
  ACE128.h - Bourns Absolute Contacting Encoder
  Copyright (c) 2013,2015,2017,2018 Alastair Young.
  This project is licensed under the terms of the MIT license.
*/

// ensure this library description is only included once
#ifndef ACE128_h
#define ACE128_h

// include types & constants of Wiring core API
#include <Arduino.h>
#include <Wire.h>
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
    // 0x00 - 0x07 MCP23008 addresses 0x20-0x27. Backward compatible with earlier library revision
    // 0x20 - 0x27 PCF8574
    // 0x38 - 0x3F PCF8574A
    // final optional eeAddr parameter sets and enables EEPROM state save for logical zero and multiturn position
    ACE128(uint8_t i2caddr, uint8_t *map);
    ACE128(uint8_t i2caddr, uint8_t *map, int16_t eeAddr);
    // direct pin constructors are similar but instead of the I2C address, you list the 8 arduino pins used
    ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map);
    ACE128(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5, uint8_t pin6, uint8_t pin7, uint8_t *map, int16_t eeAddr);
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
    uint8_t _chip;                 // chip type - derived from i2c address
    uint8_t _zero;                 // raw position of logical zero
    int8_t _reverse;               // counter-clockwise
    int _i2caddr;                  // i2c bus address
    uint8_t *_map;                 // pointer to PROGMEM map table
    int16_t _mpos;                 // multiturn offset
    int8_t _lastpos;               // last upos
    int16_t _eeAddr;               // multiturn save location (2 bytes)
    int8_t _raw2pos(int8_t pos);   // convert rawPos() value to pos()
    uint8_t _pins[8];              // store pins for direct attach mode
};


// MCP23008 IO expander
#define ACE128_MCP23008_ADDRESS 0x20
#define ACE128_MCP23008_IODIR 0x00
#define ACE128_MCP23008_IPOL 0x01
#define ACE128_MCP23008_GPINTEN 0x02
#define ACE128_MCP23008_DEFVAL 0x03
#define ACE128_MCP23008_INTCON 0x04
#define ACE128_MCP23008_IOCON 0x05
#define ACE128_MCP23008_GPPU 0x06
#define ACE128_MCP23008_INTF 0x07
#define ACE128_MCP23008_INTCAP 0x08
#define ACE128_MCP23008_GPIO 0x09
#define ACE128_MCP23008_OLAT 0x0A

// PCF8574 family
#define ACE128_PCF8574_ADDRESS 0x20
#define ACE128_PCF8574A_ADDRESS 0x38

// Arduino pins direct - no I2C
#define ARDUINO_PINS 0xff

#endif // ACE128_h

