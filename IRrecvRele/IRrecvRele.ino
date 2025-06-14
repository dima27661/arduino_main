

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
 

#include <Wire.h>
#include <IRremote.h>
#include "TM1637.h"
#include "DHT3.h"  //library  dht sensor Temperature, Humidity
#include <LiquidCrystal_I2C.h>   //library 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display 
#define DS1307_ADDRESS 0x68
#include "OneWire.h"
#include "DS18B20.h"
#include "my_functions.h"





//Массив, содержащий время компиляции
char compileTime[] = __TIME__;
char compileDate[] = __DATE__;
  byte hour = 11;
  byte minute = 23;
  byte minute_compile = 23;  
  byte second = 50; 
  byte _second = 50;  
float celsius =0;
float celsiusLimit =22;
float celsiusLimitTemp;
  int weekDay = 0; //0-6 -> sunday - Saturday
//  int monthDay = 1;
//  int month = 1;
//  int year = 2000;

  byte DS18B20addr[8];
  byte DS18B20data[12];  

// Use the built-in LED
#define LED_PIN 13

// Create a Led object
// This will set the pin to OUTPUT



//I2C. аналоговые пины для Arduino 328
//SDA -- A4 
//SCL -- A5
 
//Номера пинов Arduino, к которым подключается индикатор
#define DISPLAY_CLK_PIN 5 // CLK 
#define DISPLAY_DIO_PIN 4 //DIO 


 
//Для работы с микросхемой часов и индикатором мы используем библиотеки
//Классы TM1637 и DS1307 объявлены именно в них
TM1637 display(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);
//DS1307 clock;
DHT dht;
DS1820 DS_MY; 
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
  DateStruct _DateStruct;
  DateStruct _DateStruct_compile;  

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  DS_MY.lcd = &lcd;
  DS_MY.init(6) ;  // on pin 6 (a 4.7K resistor is necessary)



 
  IsClockOn = false;
  IsTVBoxOn = false;  
  IsLampOn = false;  
  IsSetUpOn = false;

  pinMode(PinNuber, OUTPUT);
  pinMode(12, OUTPUT); // IsTVBoxOn
  pinMode(11, OUTPUT); // lamp   
  pinMode(10, OUTPUT); //  мигаю светодиодом показываю что дистанционка работает  
  pinMode(9, OUTPUT); //  показываю что включил часы  
  pinMode(7, OUTPUT); //  жалюзи закр
  pinMode(8, OUTPUT); //  жалюзи откр  
  pinMode(LED_BUILTIN, OUTPUT);  
  
  digitalWrite(PinNuber, LOW); 
  digitalWrite(12, LOW); 
  digitalWrite(11, LOW);  
  digitalWrite(10, LOW); 
  digitalWrite(9, LOW);   
  digitalWrite(7, LOW); 
  digitalWrite(8, LOW);   
   
  lcd.init();                     
  lcd.backlight();
  lcd.setCursor(0, 0); //set Starting position

 //lcd.setCursor(1, 0); //set Starting position

  DS_MY.PrintString("Limit temp = ");
  DS_MY.PrintFloat (celsiusLimit);
  irrecv.enableIRIn(); // Start the receiver
 celsiusLimitTemp = celsiusLimit;
   
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
           
         }
      else
       {
         display.set(16);
        }  


//  clock.getTime();
GetTime(); //Запрашиваю расчётное время при отсутствии часов DS1307

timeDisp[0] = hour / 10;
timeDisp[1] = hour % 10;  //остатка от деления
timeDisp[2] = minute / 10;
timeDisp[3] = minute % 10;

  display.display(timeDisp);
   
  // вкл-выкл двоеточие каждую секунду
  display.point(_second % 2 ? POINT_ON : POINT_OFF);

 //printDateToLcd();
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
 


  if (irrecv.decode()) {
    Serial.println (irrecv.decodedIRData.decodedRawData, HEX);
  digitalWrite(10, HIGH); // мигаю светодиодом показываю что дистанционка работает 
    key_code = irrecv.decodedIRData.decodedRawData;
    
    if (key_code>0){
      ProcessCommand(key_code); // Обрабатываю код из ИК приёмника или COM порта 
    } 
  
 

    irrecv.resume(); // Receive the next value

   // sendCode(true);

    delay(300); // Wait a bit between retransmissions
     digitalWrite(10, LOW);
    irrecv.enableIRIn(); // Start the receiver
  }
  
  
  
   digitalWrite(7, LOW);    // turn the louvers  off by making the voltage LOW    
   digitalWrite(8, LOW);    // turn the louvers off by making the voltage LOW  
   incomingByte =0;
  
} // end loop


// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw

int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state


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



byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}


void printDateToLcd(){
  // Reset the register pointer

lcd.setCursor(0, 0); //set Starting position
 if (_DateStruct.day >10) {
  lcd.print( (int)( _DateStruct.day / 10) );
  lcd.print( (int)( _DateStruct.day % 10) );
 }
 else {
  lcd.print("0");
  lcd.print( _DateStruct.day);
 }
  lcd.print("/");

 if (_DateStruct.month >10) {
  lcd.print( (int)( _DateStruct.month / 10) );
  lcd.print( (int)( _DateStruct.month % 10) );
 }
 else {
  lcd.print("0");
  lcd.print( _DateStruct.month);
 }
 
  lcd.print("/");  
  lcd.print( (int)( (_DateStruct.year % 100) /10 ) );
  lcd.print( (int) (_DateStruct.year % 10)   ); 
 
  lcd.setCursor(0, 1); //set Starting position

 if (hour >10) {
  lcd.print( (int)( hour / 10) );
  lcd.print( (int)( hour % 10) );
 }
 else {
  lcd.print("0");
  lcd.print( hour);
 }
  
  lcd.print(":");

 if (minute >10) {
  lcd.print( (int)( minute / 10) );
  lcd.print( (int)( minute % 10) );
 }
 else {
  lcd.print("0");
  lcd.print( minute);
 }
   
  lcd.print(":");

 if (second >10) {
  lcd.print( (int)( second / 10) );
  lcd.print( (int)( second % 10) );
 }
 else {
  lcd.print("0");
  lcd.print( second);
 }
 
  lcd.print(" ");

}



void PrintTemperature()
{

if ((millis() - sensor_sleep_time) > 5000) {


   Serial.println( sensor_sleep_time);
 //  Serial.println( dht.getMinimumSamplingPeriod());
   sensor_sleep_time = millis();

   // DS_MY.PrintOutTemperature();
   // DS_MY.read_1(); //ошибка нужно курить https://medium.com/@rishabhdevyadav/create-your-own-arduino-library-h-and-cpp-files-62ab456453e0
   // DS_MY.PrintInTemperature();   
  celsius =  DS_MY.PrintAll(); // покажет все градусники на этом пине

  
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  if  (celsius < celsiusLimitTemp){
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      IsLampOn =true; 
           lcd.setCursor(0, 1); //set Starting position
                 DS_MY.PrintString("Power ON ");
                 
      if (celsiusLimitTemp <= celsiusLimit) {
          Serial.print("   celsius = ");
          Serial.print(celsius);
          Serial.print("   celsiusLimitTemp = ");
          Serial.print(celsiusLimitTemp);
          Serial.print("   celsiusLimit = ");
          Serial.print(celsiusLimit);         
          celsiusLimitTemp = celsiusLimit+1;
         
      }
  }
         else {
      if (celsiusLimitTemp >= celsiusLimit) {
         celsiusLimitTemp = celsiusLimit-1;
         
      }          
         IsLampOn =false; 
         lcd.setCursor(0, 1); //set Starting position
         DS_MY.PrintString("Power OFF");         
       }

  
     lcd.setCursor(13, 0); //set Starting position
                // DS_MY.PrintString("Limit temp = ");
                 DS_MY.PrintFloat (celsiusLimitTemp);
  
  Serial.print("  Temperature celsius = ");
  Serial.print(celsius);
  
   int result = addTwoInts(4,3); // работает из внешнего файла


Serial.println (" ");
  Serial.print (  _DateStruct.day);
  Serial.print("/");
  Serial.print(_DateStruct.month);
  Serial.print("/");  
  Serial.print(_DateStruct.year); 
  Serial.print(" "); 
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
   
    
 }     
}




void ProcessCommand (  unsigned long key_code){
  
 //   storeCode(&results);
     Serial.println (key_code); 


         if  (key_code == 4161210119 || key_code == 4094426880) // громкость +
          {celsiusLimit++;celsiusLimitTemp =celsiusLimit;} 
         if  (key_code == 4094363399 || key_code == 4077715200) // громкость -
          {celsiusLimit--;celsiusLimitTemp =celsiusLimit;}   
               if  (key_code == 4161210119 || key_code == 4094363399 || key_code == 4077715200 || key_code == 4094426880)//громкость +-
               {
                 lcd.setCursor(0, 0); //set Starting position
                 DS_MY.PrintString("Limit temp = ");
                 DS_MY.PrintFloat (celsiusLimitTemp);
               }
 
    if  (key_code == 16642 || key_code == 50) // 2 на пульте
    {
     Serial.println (key_code);       
       if (IsTVBoxOn) {
         IsTVBoxOn =false;
         }
       else {
         IsTVBoxOn =true; 
       }
     }   
     
    
     

    if  (key_code == 17806) // MENU на пульте
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
         if  (key_code == 16546) // громкость +
          {hour++;} 
         if  (key_code == 17058) // громкость -
          {hour--;}   
         if  (key_code == 16930) // p +
          {minute++;  second =0;} 
         if  (key_code == 16674) // p -
          {minute--;  second =0;}  
         if  (key_code == 16898) // 1 на пульте
          {_DateStruct.day++;  }
         if  (key_code == 16642 ) // 2 на пульте          
          {_DateStruct.day--;  }
         if  (key_code == 17154   ) // 51-ascii =  3 на пульте   
          {_DateStruct.month++;  }                
         if  (key_code == 16514   ) // 4 на пульте   
          {_DateStruct.month--;  }   
         if  (key_code == 17026   ) // 5 на пульте   
          {_DateStruct.year++;  } 

         if  (key_code == 16770   ) // 6 на пульте   
          {_DateStruct.year--;  } 



        
        // _DateStruct.day, _DateStruct.month, _DateStruct.year          

          

     }   
    else
     {
         if  (key_code == 16930) // p +
          {     
             digitalWrite(7, HIGH);  
           //  delay(500);              // wait for a second
           //  digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW
           } 
         if  (key_code == 16674) // p -
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
