#include "Led.h"

Led::Led(byte pin) {
  this->pin = pin;
  init();
}

void Led::init() {
  pinMode(pin, OUTPUT);
 // OneWire  ds(pin);  // on pin 6 (a 4.7K resistor is necessary)
  off();
}

void Led::on() {
  digitalWrite(pin, HIGH);
   // ds.reset();
}

void Led::off() {
  digitalWrite(pin, LOW);
}

