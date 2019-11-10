/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff


// use IR_USE_TIMER1 to resolve timer conflict  when use tone() function else use  IR_USE_TIMER2
// #else
//  #define IR_USE_TIMER1   // tx = pin 9
//  //#define IR_USE_TIMER2     // tx = pin 3
//#endif
 
 * http://arcfn.com

 */
#include <IRremote.h>
#include <Wire.h>
#include "TM1637.h"
#include "DHT3.h"  //library  dht sensor Temperature, Humidity
#include <LiquidCrystal_I2C.h>   //library 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#define DS1307_ADDRESS 0x68
#include <OneWire.h>

//Массив, содержащий время компиляции
char compileTime[] = __TIME__;
char compileDate[] = __DATE__;
  byte hour = 11;
  byte minute = 23;
  byte second = 50; 
  byte _second = 50;  

  int weekDay = 0; //0-6 -> sunday - Saturday
  int monthDay = 1;
  int month = 1;
  int year = 2000;

  byte DS18B20addr[8];
  byte DS18B20data[12];  
OneWire  ds(6);  // on pin 6 (a 4.7K resistor is necessary)

//I2C. аналоговые пины для Arduino 328
//SDA -- A4 
//SCL -- A5
 
//Номера пинов Arduino, к которым подключается индикатор
#define DISPLAY_CLK_PIN 3 // CLK 
#define DISPLAY_DIO_PIN 4 //DIO 


 
//Для работы с микросхемой часов и индикатором мы используем библиотеки
//Классы TM1637 и DS1307 объявлены именно в них
TM1637 display(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);
//DS1307 clock;
DHT dht;
 
int PinNuber;

boolean IsClockOn;
boolean IsTVBoxOn;
boolean IsLampOn;
boolean IsSetUpOn;
int RECV_PIN = 2; // /Номер пина Arduino, к которым подключается ИК приёмник
unsigned long time0; // объявление переменной time типа unsigned long
unsigned long time_interval;
unsigned long rele_sleep_time; 
unsigned long sensor_sleep_time; // время последнего опроса сенсора
unsigned long start_time0; // 
unsigned long min_interval;
byte rest_hour=0; 
 
 unsigned long key_code;// код кнопки "1" на пульте = 2287677653

IRrecv irrecv(RECV_PIN);
decode_results results;
int incomingByte = 0;

void setup()
{
  Wire.begin();
  
  display.set();
  display.init();

   hour = getInt(compileTime, 0);
   minute = getInt(compileTime, 3);
   second = getInt(compileTime, 6);


   _second = second;
   start_time0 = millis();
   min_interval = millis() - start_time0;
   min_interval = _second * 1000;
  
  rele_sleep_time = 9000; //  время (для часов 9000) сколько реле разомкнуто
  IsClockOn = false;
  IsTVBoxOn = false;  
  IsLampOn = false;  
  IsSetUpOn = false;
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
    PinNuber = 13; // реле (большое) часов    
  pinMode(PinNuber, OUTPUT);
  pinMode(12, OUTPUT); // IsTVBoxOn
  pinMode(11, OUTPUT); // lamp   
  pinMode(10, OUTPUT); //  мигаю светодиодом показываю что дистанционка работает  
  pinMode(9, OUTPUT); //  показываю что включил часы  
  pinMode(7, OUTPUT); //  жалюзи закр
  pinMode(8, OUTPUT); //  жалюзи откр  
  dht.setup(5); //DHT3 сенсор Temperature , Humidity.  5 pin  
  
  digitalWrite(PinNuber, LOW); 
  digitalWrite(12, LOW); 
  digitalWrite(11, LOW);  
  digitalWrite(10, LOW); 
  digitalWrite(9, LOW);   
  digitalWrite(7, LOW); 
  digitalWrite(8, LOW);   

   
  time0 = millis();
  time_interval = millis() - time0;
  sensor_sleep_time = time_interval;
  lcd.init();                     
  lcd.backlight();
  lcd.setCursor(0, 0); //set Starting position



  delay(dht.getMinimumSamplingPeriod()); 
    
}




void loop() {

  //Значения для отображения на каждом из 4 разрядов
  int8_t timeDisp[4];
  
  unsigned long time; // объявление переменной time типа unsigned long
  time = time_interval;   // передача количества миллисекунд


SecCount(); // счётчик секунд независит от часов DS1307

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    key_code = incomingByte;
     ProcessCommand(key_code); // Обрабатываю код из ИК приёмника или COM порта 
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }


  if (IsSetUpOn) {
         //  display.set(20); // ярче  
           display.set(_second % 2 ? 16 : 8);
          // setDateTimeDS1307(byte (second),byte (minute),byte (hour), byte weekDay,byte monthDay,byte month,byte year);
           setDateTimeDS1307(byte (second),byte (minute),byte (hour),4,5,10,2017);
         }
      else
       {
         display.set(16);
        }  



//  clock.getTime();
//GetTime(); //Запрашиваю расчётное время при отсутствии часов DS1307

GetTimeDS1307(); //Запрашиваю время из часов DS1307

timeDisp[0] = hour / 10;
timeDisp[1] = hour % 10;  //остатка от деления
timeDisp[2] = minute / 10;
timeDisp[3] = minute % 10;

  display.display(timeDisp);
   
  // вкл-выкл двоеточие каждую секунду
  display.point(_second % 2 ? POINT_ON : POINT_OFF);

 printDateToLcd();
 PrintTemperature();


    
time_interval = millis() - time0;

   if (IsClockOn) {
     
     if (time_interval > rele_sleep_time) { 
      digitalWrite(PinNuber, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(200);              // wait for a second
      digitalWrite(PinNuber, LOW);    // turn the LED off by making the voltage LOW
      time0 = millis(); 
    }
  }
 
    if (IsTVBoxOn) {digitalWrite(12, HIGH);} 
    else { digitalWrite(12, LOW);}
 
     if (IsLampOn) {digitalWrite(11, HIGH);} 
    else { digitalWrite(11, LOW);}
 


  if (irrecv.decode(&results)) {
    Serial.println (results.value, HEX);
  digitalWrite(10, HIGH); // мигаю светодиодом показываю что дистанционка работает 
    key_code = results.value;

    ProcessCommand(key_code); // Обрабатываю код из ИК приёмника или COM порта 
  
 

    irrecv.resume(); // Receive the next value

   // sendCode(true);

    delay(300); // Wait a bit between retransmissions
     digitalWrite(10, LOW);
    irrecv.enableIRIn(); // Start the receiver
  }
  
  
  
   digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW    
   digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW  
   incomingByte =0;
  
} // end loop


// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    key_code = results->value;
    codeLen = results->bits;
  }
}
//Содержимое функции объяснено ниже
char getInt(const char* string, int startIndex) {
  return int(string[startIndex] - '0') * 10 + int(string[startIndex+1]) - '0';
}

void GetTime() {
    min_interval = millis() - start_time0;
  second = min_interval / 1000 ;
  if (second > 59) {
     second = 0;
     minute = minute +1;
     start_time0 = millis() + 25; // добавил 50  = 1400 мин в сутки * 50 = 72000 милисек в сутки,  чтоб отставали
  //   delay(10); // отставали раньше
    }

  if (minute > 59) {
     minute = 0;
     hour = hour + 1;
    }

  if (hour > 23) {
     hour = 0;
    } 
}

void SecCount() {
    min_interval = millis() - start_time0;
  _second = min_interval / 500 ;
  if (_second > 59) {
     _second = 0;
     start_time0 = millis();
    }
}

void GetTimeDS1307() {
  Wire.beginTransmission(DS1307_ADDRESS);

  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);

  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
   weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
   monthDay = bcdToDec(Wire.read());
   month = bcdToDec(Wire.read());
   year = bcdToDec(Wire.read());

}



byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

void setDateTimeDS1307(byte second,byte minute,byte hour, byte weekDay,byte monthDay,byte month,byte year){

//  byte second =      00; //0-59
//  byte minute =      33; //0-59
//  byte hour =        23; //0-23
//  byte weekDay =     4; //1-7
//  byte monthDay =    05; //1-31
//  byte month =       11; //1-12
//  byte year  =       16; //0-99
byte zero = 0x00; //workaround for issue #527
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));

  Wire.write(zero); //start 
  Wire.endTransmission();

}

void printDateToLcd(){
  // Reset the register pointer

//lcd.setCursor(0, 0); //set Starting position
//  lcd.print(monthDay);
//  lcd.print("/");
//  lcd.print(month);
//  lcd.print("/");  
//  lcd.print(year);

//  lcd.print(" ");
  lcd.setCursor(0, 1); //set Starting position
  if (hour<10){ lcd.print("0");} 
  lcd.print(hour);
  lcd.print(":");
   if (minute<10){ lcd.print("0");}   
  lcd.print(minute);
  lcd.print(":");
   if (second<10){ lcd.print("0");}     
  lcd.print(second);
  lcd.print(" ");

//  if (monthDay<10){ lcd.print("0");} 
//  lcd.print(monthDay);
//  lcd.print("/");
//   if (month<10){ lcd.print("0");}   
//  lcd.print(month);
//  lcd.print("/");
  
  // if (second<10){ lcd.print("0");}     
  //lcd.print(year);
  
  
}



void PrintTemperature()
{

if ((millis() - sensor_sleep_time) > 30000) {
  int tem;
  int hum;  
  tem = dht.getTemperature();
  hum = dht.getHumidity();
  if (tem > 0) {
    lcd.setCursor(0, 0); //set Starting position  
  //  lcd.print("                ");  
    lcd.setCursor(0, 0); //set Starting position   

       
  //   lcd.print(tem);
  //   lcd.print("C");     
  //   lcd.print(" h-"); 
  //   lcd.print(hum);   
 //    lcd.print("%");  

  if (monthDay<10){ lcd.print("0");} 
  lcd.print(monthDay);
  lcd.print("/");
   if (month<10){ lcd.print("0");}   
  lcd.print(month);
  lcd.print("/17");   

         
    }
   Serial.println( sensor_sleep_time);
 //  Serial.println( dht.getMinimumSamplingPeriod());
   sensor_sleep_time = millis();

    PrintOutTemperature();
    PrintInTemperature();    
 }     
}


void PrintInTemperature()
{
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

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
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
     lcd.setCursor(10, 1); //set Starting position       
     lcd.print(celsius);
     lcd.print("C");     
}


void PrintOutTemperature()
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
 // byte addr[8];
  byte addr[8]={0x28, 0xFF, 0xEF, 0xAF, 0xB3, 0x16, 0x3, 0x32}; //градусник на улице

//  byte addr[8]={0x28, 0xFF, 0xE5, 0xA, 0x33, 0x17, 0x3, 0x1}; //градусник внутри 2  
  
  float celsius, fahrenheit;
  
  // if ( !ds.search(addr)) {
  //  Serial.println("No more addresses.");
  //  Serial.println();
  //  ds.reset_search();
  //  //delay(250);
  //  return;
 // }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

     lcd.setCursor(10, 0); //set Starting position       
         
     lcd.print(celsius);
     lcd.print("C");     
   //  lcd.print(" h-"); 
   //  lcd.print(hum);   
   //  lcd.print("%"); 
 
}

void ProcessCommand (  unsigned long key_code){
  
 //   storeCode(&results);
     Serial.println (key_code); 
   if  (key_code == 2287677653) // 1 на пульте
    {
     Serial.println (key_code);       
       if (IsClockOn) {
         IsClockOn =false;
         digitalWrite(9,LOW);
         }
       else {
         IsClockOn =true; 
         digitalWrite(9, HIGH);
       }
     } 
 
    if  (key_code == 3890270129) // 2 на пульте
    {
     Serial.println (key_code);       
       if (IsTVBoxOn) {
         IsTVBoxOn =false;
         }
       else {
         IsTVBoxOn =true; 
       }
     }   
     
   if  (key_code == 2739886593 || key_code == 51  ) // 51-ascii =  3 на пульте
    {
     Serial.println (key_code);       
       if (IsLampOn) {
         IsLampOn =false;
         }
       else {
         IsLampOn =true; 
       }
     }      
     

    if  (key_code == 3353394895) // MENU на пульте
    {
     Serial.println (key_code);       
       if (IsSetUpOn) {
         IsSetUpOn =false;
         }
       else {
         IsSetUpOn =true;          
       }
     }   
       
    if  (IsSetUpOn) // set time from du
    {
         if  (key_code == 1259510059) // громкость +
          {hour++;} 
         if  (key_code == 468240397) // громкость -
          {hour--;}   
         if  (key_code == 2970385547) // p +
          {minute++;  second =0;} 
         if  (key_code == 278010727) // p -
          {minute--;  second =0;}  

     }   
    else
     {
         if  (key_code == 2970385547) // p +
          {     
             digitalWrite(7, HIGH);  
           //  delay(500);              // wait for a second
           //  digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW
           } 
         if  (key_code == 278010727) // p -
          {
             digitalWrite(8, HIGH);  
           //  delay(500);              // wait for a second
          //   digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW
          }        
       
     }    
     
     
   // key_code = 0;  
}

       

   // delay(2000);




//------------------------------------------------------------------------
//------------------------------------------------------------------------







