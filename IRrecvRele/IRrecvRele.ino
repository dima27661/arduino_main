
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
  byte second = 50; 
  byte _second = 50;  

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
#define DISPLAY_CLK_PIN 3 // CLK 
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

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  DS_MY.lcd = &lcd;
  DS_MY.init(6) ;  // on pin 6 (a 4.7K resistor is necessary)

  display.set();
  display.init();

   hour = getInt(compileTime, 0);
   minute = getInt(compileTime, 3);
   second = getInt(compileTime, 6);

  _DateStruct = ConvertDate (__DATE__);
 
  weekDay = 1;

  setDateTimeDS1307(byte (second),byte (minute),byte (hour),  weekDay,  _DateStruct.day, _DateStruct.month, _DateStruct.year);

   _second = second;
   start_time0 = millis();
   min_interval = millis() - start_time0;
   min_interval = _second * 1000;
  
  rele_sleep_time = 9000; //  время (для часов 9000) сколько реле разомкнуто
  IsClockOn = false;
  IsTVBoxOn = false;  
  IsLampOn = false;  
  IsSetUpOn = false;

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
           setDateTimeDS1307(byte (second),byte (minute),byte (hour), 4,_DateStruct.day, _DateStruct.month, _DateStruct.year);
          // setDateTimeDS1307(byte (second),byte (minute),byte (hour),4,5,10,2017);
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
  
  
  
   digitalWrite(7, LOW);    // turn the louvers  off by making the voltage LOW    
   digitalWrite(8, LOW);    // turn the louvers off by making the voltage LOW  
   incomingByte =0;
  
} // end loop


// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
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
   
   _DateStruct.day = bcdToDec(Wire.read());
   _DateStruct.month = bcdToDec(Wire.read());
   _DateStruct.year = bcdToDec(Wire.read());

}



byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

void setDateTimeDS1307(byte second,byte minute,byte hour, byte weekDay,byte monthDay,byte month,int year){

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


lcd.setCursor(0, 0); //set Starting position
  lcd.print(_DateStruct.day);
  lcd.print("/");
  lcd.print(_DateStruct.month);
  lcd.print("/");  
  lcd.print(_DateStruct.year);

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

  if (_DateStruct.day<10){ lcd.print("0");} 
  lcd.print(_DateStruct.day);
  lcd.print("/");
   if (_DateStruct.month<10){ lcd.print("0");}   
  lcd.print(_DateStruct.month);
  lcd.print("/");
  lcd.print(_DateStruct.year);   

         
    }
   Serial.println( sensor_sleep_time);
 //  Serial.println( dht.getMinimumSamplingPeriod());
   sensor_sleep_time = millis();

   // DS_MY.PrintOutTemperature();
   // DS_MY.read_1(); //ошибка нужно курить https://medium.com/@rishabhdevyadav/create-your-own-arduino-library-h-and-cpp-files-62ab456453e0
  //  DS_MY.PrintInTemperature();   
    DS_MY.PrintAll(); // покажет все градусники на этом пине
   int result = addTwoInts(4,3); // работает из внешнего файла
    
 }     
}
// -----------------------

// -----------------------




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
 
    if  (key_code == 3890270129 || key_code == 50) // 2 на пульте
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
