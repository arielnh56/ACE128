// make_encodermap for ACE128 library
// To make life easier designing PCBs for the ACE encoder, you can arrange the pins in any order
// we just need to make a different mapping table
// the example spits out content on the serial monitor that can be pasted into a text editor to
// make the alternate map table .h file

// edit pinOrder and pinString for your encoder pins from pin 0 to 7 on your IO expander
byte pinOrder[] = {1, 2, 3, 4, 5, 6, 7, 8};   // used for calculation
char pinString[] = "12345678";                // used for naming things

void setup() {  
  int i,j; // loop vars
  // track binary data taken from p1 column on datasheet
  byte track[] = {B11000000, B00111111, B11110000, B00001111, B11100000, B00011111, B11111111, B11111111, 
                  B11111111, B00000000, B11111100, B00000011, B10000000, B01111000, B00000110, B00000001 };
  byte encoderMap[256];  // an array of all possible bit combinations
  for (i=0; i<=255; i++) encoderMap[i] = 255;  // initialize to invalid value 
  byte mask;  // bit finder
  byte index; // index into encoderMap
  int pos;    // current position
  for (pos = 0; pos <= 127; pos ++) {       // imagine rotating the encoder
    mask = B10000000 >> pos % 8;            // which bit in current byte
    index = 0;                              // start with a blank slate
    for (int pin=0; pin <= 7; pin++) {      // think about each pin
      // which byte in track[] to look at. Each pin is 16 bits behind the previous
      int offset = (( pos - ( ( 1 - pinOrder[pin] ) * 16 )) % 128 ) / 8; // which byte in track[]
      if ( track[offset] & mask ) {     // is the bit set?
        index |= (1 << pin);            // set that pin's bit in the index byte
      }   
    }
    encoderMap[index] = pos;           // record the position in the map
  }

  // spit out our include file 
  Serial.begin(9600);
  Serial.print("// ACE128map");
  Serial.print(pinString);
  Serial.print(".h\n");
  Serial.print("// ACE128 encoder map for pin order ");
  Serial.println(pinString);
  Serial.print("#ifndef ACE128MAP");
  Serial.print(pinString);
  Serial.print("_H_INCLUDED\n");
  Serial.print("#define ACE128MAP");
  Serial.print(pinString);
  Serial.print("_H_INCLUDED\n");
  Serial.print("#include <avr/pgmspace.h>\n\n");
  Serial.print("PROGMEM extern const prog_uchar encoderMap_");
  Serial.print(pinString);
  Serial.print("[256] = {\n  ");
  for (i=0; i<16; i++) {        // 16 lines
    for (j=0; j<16; j++) {      // 16 per line
      index=i*16 + j;           // which one to print
      pos=encoderMap[index];    // map value
      Serial.print("0x");       // hex prefix
      if (pos < 0x10) Serial.print("0");  // leading 0 on small numbers for prettiness   
      Serial.print(pos,HEX);              // print the value
      if (index < 255) Serial.print(","); // no comma after the last item
      
    }
    if (i < 15) Serial.print("\n  ");    // no new line after the last line
  }
  Serial.print(" };\n\n");     // close
  Serial.print("#endif\n");    // bye
}

void loop() {
}
