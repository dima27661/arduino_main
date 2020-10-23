#ifndef LED_H
#define LED_H

#include <Arduino.h>

class Led {
  
  private:
    byte pin;
 //  OneWire  ds(6);  // on pin 6 (a 4.7K resistor is necessary)
    
    
  public:
    // Setup pin LED and call init()

    Led(byte pin);
    

    // Setup the pin led as OUTPUT
    // and power off the LED - default state
    void init();
    
    // Power on the LED
    void on();

    // Power off the LED
    void off();
};

#endif
