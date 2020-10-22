
 #include <Arduino.h>
 #include <OneWire.h>
 OneWire  ds1(6);  // on pin 6 (a 4.7K resistor is necessary)
class DS1820 {  
    private:
    byte i;
   //  byte present = 0;
   //  byte type_s;
   //  byte data[12];
   //  byte addr[8]={0x28, 0xFF, 0xEF, 0xAF, 0xB3, 0x16, 0x3, 0x32}; //градусник на улице
   // byte addr[8]={0x28, 0xFF, 0xE5, 0xA, 0x33, 0x17, 0x3, 0x1}; //градусник внутри 2  

 

  public:
   int pin;  // переменная с номером пина
   int data_test; // переменная с показаниями датчика
  
  
  void DS1820::read_1()  // добавляем функцию read
       {
        //  data=analogRead(pin);  // считываем показания датчика в переменную data
         data_test=1;
       } 

void PrintInTemperature()
{
  String myStr;
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  //  byte addr[8]={0x28, 0xFF, 0xEF, 0xAF, 0xB3, 0x16, 0x3, 0x32}; //градусник на улице
  byte addr[8]={0x28, 0xFF, 0xE5, 0xA, 0x33, 0x17, 0x3, 0x1}; //градусник внутри 2  
  
  float celsius, fahrenheit;
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
 
  // the first ROM byte indicates which chip
    type_s = 0;

  ds1.reset();
  ds1.select(addr);
  ds1.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  
  present = ds1.reset();
  ds1.select(addr);    
  ds1.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds1.read();
  }

  int16_t raw = (data[1] << 8) | data[0];

    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

  //   lcd.setCursor(10, 1); //set Starting position       
  //   lcd.print(celsius);
  //   lcd.print("C");     
}


       
};

