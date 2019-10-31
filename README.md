![Bourns ACE-128](http://www.bourns.com/images/Products/encoders/dataprodline_sensors_controls_enc_contact.jpg)

This library provides support for the Bourns EAW - Absolute Contacting Encoder (ACE) 

Datasheet: https://www.bourns.com/pdfs/ace.pdf

This is a small 8 bit "Gray Code" rotary position sensor providing 128 unique positions. It manages this with an [ingeniously simple mechanical design](http://redhunter.com/blog/2017/11/25/inside-the-bourns-ace-128-absolute-encoder/).

In layman's terms this is a "digital knob" you can use to control your Arduino project. It can
also be used in motion control systems with a limited lifespan.

This code is developed against the Arduino UNO R3 and Arduino MKR Wifi 1010. 

It should work on any Arduino without modification to drive the sensor via my PCF8574(A) backpack modules via the I2C bus. The I2C bus must be correctly wired and include appropriate pullup resistors.

For basic testing of the manufactured modules use the ace128_0x20 and ace128_0x38 examples. These will send output to the serial monitor at 9600 baud.

See the ACE128test example sketch for detailed usage instructions for all the options.

By default the code supports **only** the PCF8574 and PCF8574A I2C expanders, as used in the author's manufactured modules, and can
optionally save state to EEPROM on AVR microcontrollers. By default it *does not save state on SAM microcontrollers* as they do
not have EEPROM - this requires a separate I2C EEPROM chip and tweaking the library to turn on support for this feature. 

The following features can be enabled via uncommenting #defines in the ACE128.h include file:
* use of MCP23008 pin expander
* use of Arduino pins to talk directly to the Bourns encoder. This disables the pin expander code.
* use of I2C EEPROMs to save state. These have longer life than the AVR EEPROM and provide storage for the SAM microcontrollers.
* the ability to disable the state saving code altogether and save some flash memory.

Full instruction on these feature are in the library comments.

If the IDE is giving you grief editing ACE128.h then copy it to your sketch directory and use

```c++
#include "ACE128.h"
```

with quotes instead of angle brackets. That will make it use your local copy.

Installation
--------------------------------------------------------------------------------

This library conforms to the library manager standard, so it needs no special instructions here.
See https://www.arduino.cc/en/Guide/Libraries

Basic Usage
--------------------------------------------------------------------------------
See the ACE128test, ace128_0x20 and ace128_0x38 example sketches.

Include the library, either from the library folder with
```c++
#include <ACE128.h>
```
or copy to your sketch folder for easier editing of feature flags and use
```c++
#include "ACE128.h"
```
Note that all of the code is included in the ACE128.h file - there is no ACE128.cpp file any more. This was necessary to implement the feature flags in the Arduino IDE environment. (ask me over a beer)

* Include all the encoder maps you need (see below) to match the pin sequences of your ACE units. 
    * the manufactured modules all use the 87654321 encoder map as in the examples
* Declare all your ACE128 objects using the ACE128 constructor. It takes an I2C address and a pointer to the encoder map.
    An optional third parameter can take a positive integer to show where to store zero info in eeprom. Allow for three bytes
    for each module.
* call the begin() method for each ACE128 object from setup(). This will use the eeprom settings or fall back to setting the current position as logical zero.
* The pos() and upos() methods return the position relative to a logical zero
position rather than the zero position returned by the encoder, which is in a
mechanically arbitrary spot. When it rolls over the turns are stored for use by mpos and saved in eeprom
* Once per loop(): call pos(), upos(), or mpos() and store the value in a variable. Accessing the I2C bus takes some cycles,
so don't call pos() everytime you want to refer to it. 
* there are three setting functions
    * setZero()   - set the current location to zero (does not update multiturn)
    * setZero(int)   -  sets the zero point to the 0-127 number given
    * setMpos(int)    - sets the current location as this multiturn value. This adjust logical zero appropriately.
    * reverse(bool) - if true, makes it rising anticlockwise

Encoder Maps
--------------------------------------------------------------------------------

This library comes with various encoder maps to permit different wiring of the pins
between the ACE and the IO expander. These are useful if you are embedding the sensor in your own circuit design. The author's modules all use the 87654321 map.
Custom encoder maps are generated by the included make_encodermap example sketch as follows:
* edit the make_encodermap example sketch
* change the pinOrder array to match your wiring
* change the pinString to match your wiring
* load the sketch to the Arduino
* copy the serial monitor output to a new .h file in the ACE128 folder.  

12345678 is for the "rising counter clockwise" wiring, which matches the datasheet
numbers and is recommended for breadboard testing. 
When breadboarding, remember the pins on the sensor are numbered anticlockwise as viewed from above.

Licensing
--------------------------------------------------------------------------------

The contents of this repo are published under the MIT License. This includes the enclosed hardware designs. The project is certified as open source hardware by the Open Source Hardware Association. 

<img src="ace128-oshw-US000172.png" alt="US000172" width="200"/>

comments and feedback via https://github.com/arielnh56/ACE128

more details and videos at https://www.tindie.com/products/8759/

buy assembled units at https://www.tindie.com/products/8759/ https://www.tindie.com/products/18118/
