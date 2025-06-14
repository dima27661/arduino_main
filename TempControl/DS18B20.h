 #include <inttypes.h>
 #include <Arduino.h>
 #include "OneWire.h"
 #include <LiquidCrystal_I2C.h>   //library 
 class DS1820   // название класса
{
  private:
    
    byte i,CursorPos;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8]={0x28, 0xFF, 0xEF, 0xAF, 0xB3, 0x16, 0x3, 0x32}; //градусник на улице
    // byte addr[8]={0x28, 0xFF, 0xE5, 0xA, 0x33, 0x17, 0x3, 0x1}; //градусник внутри 2  
   OneWire ds;  // on pin 6 (a 4.7K resistor is necessary)

  public:
    int pin;  // данные - пин подключения датчика, по которому получаем значение датчика
    int data_test; // данные - значение датчика
    void init(uint8_t pin);
    void PrintFloat(float pin);
    void PrintString(String val);
    LiquidCrystal_I2C *lcd; 
    float PrintAll(void);
//    PrintAllStr(float celsius );
};
