#include <ACE128.h>  // Absolute Contact Encoder
#include <ACE128map12345678.h> // mapping for pin order 10_11_12_13_6_7_8_9 


#define ACE_ADDR 0x38
// address range 0x00 - 0x07 -> MCP23008 at address 0x20 -> 0x27
// address range 0x20 - 0x27 -> PCF8574  at address 0x20 -> 0x27
// address range 0x38 - 0x3F -> PCF8574A at address 0x38 -> 0x3F
// note that the PCF8574 datasheet lists these addresses twice these values
// they count the read/write bit in the address, which is a source of confusion
// Create an ACE128 instance called myACE at address 0 (really 0x20) using the basic encoder map
byte pinOrder[8] = {10, 11, 12, 13, 9, 8, 7, 6};   // used for calculation
 ACE128 myACE(pinOrder, (uint8_t*)encoderMap_12345678);
// ACE128 myACE((uint8_t)0x00, (uint8_t*)encoderMap_12345678, -1); // datasheet basics - MCP23008
//ACE128 myACE((uint8_t)ACE_ADDR, (uint8_t*)encoderMap_10_11_12_13_6_7_8_9); // backpack module



// set-zero button on pin 13
// - button to ground e.g. MakerShield button
const int ZERO = 13;

uint8_t pinPos = 0; // pin values from MCP23008 or PCF8574
uint8_t rawPos = 0;
uint8_t upos = 0;
uint8_t oldPos = 255;
int8_t pos;
int16_t mpos;
uint8_t seen = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Serial.println(myACE.acePins());
  

//  myACE.begin();    // this is required for each instance, initializes the IO expander
   myACE.begin8Pins();
  // myACE.reverse(true);  // uncomment this for counter-clockwise operation
  //pinMode(ZERO, INPUT_PULLUP);    // configure set-zero button
  //pinPos = myACE.acePins();          // get IO expander pins
  pinPos = myACE.get8pinsrawPos();          // get 8 pin position
  oldPos = pinPos;                 // remember where we are

  Serial.print("begin:");
  Serial.println(pinPos);
}

void loop() {
  // put your main code here, to run repeatedly:
    //if (digitalRead(ZERO) == 0) {      // check set-zero button
    if (false) {      // check set-zero button

    // myACE.setMpos(6723);  // set current position to 6723
    myACE.setMpos(0);    // set logical multiturn zero to current position
    // myACE.setZero();     // set logical zero to current position
    // myACE.reverse(true);  // set reverse
    oldPos = 255;                   // force display update
  }

  //pinPos = myACE.acePins();          // get IO expander pins
  pinPos = myACE.get8pinsrawPos();
  rawPos = myACE.rawPos();           // get raw mechanical position
  pos = myACE.pos();                 // get logical position - signed
  upos = myACE.upos();               // get logical position - unsigned
  mpos = myACE.mpos();               // get multiturn position - signed

    if (pinPos != oldPos) {            // did we move?
    seen |= pinPos ^ oldPos;         // what changed?
    oldPos = pinPos;                 // remember where we are
 
    if (false) {
      //if (seen < 255) {
      Serial.println("seen:");
      Serial.print(seen);
      
    } else {
      Serial.print("pin ");
      Serial.print(pinPos);
      Serial.print("raw ");
      Serial.print(rawPos);
      Serial.print("pos dec ");
      Serial.print(pos, DEC);
      Serial.print("upos dec ");
      Serial.print(upos, DEC);
      Serial.print("mpos dec ");
      Serial.println(mpos, DEC);
    }
  }
  
}
