/*
  ShiftRegister74HC595 - Library for simplified control of 74HC595 shift registers.
  Developed and maintained by Timo Denk and contributers, since Nov 2014.
  Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
  Released into the public domain.
*/

#include <ShiftRegister74HC595.h>

// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(11, 12, 8);
 
void setup() { 
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

  // setting all pins at the same time to either HIGH or LOW
  //sr.setAllHigh(); // set all pins HIGH
  //delay(500);
  
  //sr.setAllLow(); // set all pins LOW
  //delay(500); 
  

  // setting single pins
  for (int i = 0; i < 16; i++) {
    
    sr.set(i, HIGH); // set single pin HIGH
    delay(30); 
    sr.set(i, LOW); // set single pin HIGH
    delay(30);    
  }
  
  
  // set all pins at once
  //uint8_t pinValues[] = { B10101010 }; 
  //sr.setAll(pinValues); 
  //delay(1000);

  
  // read pin (zero based, i.e. 6th pin)
  //uint8_t stateOfPin5 = sr.get(5);
  //sr.set(6, stateOfPin5);


}
