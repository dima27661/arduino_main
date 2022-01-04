

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




// Use the built-in LED
#define LED_PIN 13

// Create a Led object
// This will set the pin to OUTPUT

 
int PinNuber;


boolean IsTVBoxOn;
boolean IsLampOn;

int RECV_PIN = 2; // /Номер пина Arduino, к которым подключается ИК приёмник

byte rest_hour=0; 
 
 unsigned long key_code;// код кнопки "1" на пульте = 2287677653

IRrecv irrecv(RECV_PIN);
decode_results results;
int incomingByte = 0;

void setup()
{
  Serial.begin(9600);
  


  IsTVBoxOn = false;  
  IsLampOn = false;  


  irrecv.enableIRIn(); // Start the receiver
    PinNuber = 13; // реле (большое) часов    
  pinMode(PinNuber, OUTPUT);
  pinMode(12, OUTPUT); // IsTVBoxOn
  pinMode(11, OUTPUT); // lamp   

  pinMode(9, OUTPUT); //  показываю что включил   
 
  
  // dht.setup(5); //DHT3 сенсор Temperature , Humidity.  5 pin  
  
  digitalWrite(PinNuber, LOW); 
  digitalWrite(12, LOW); 
  digitalWrite(11, LOW);  
  digitalWrite(10, LOW); 
  digitalWrite(9, LOW);   
  digitalWrite(7, LOW); 
  digitalWrite(8, LOW);   
   

    
}




void loop() {


  if (irrecv.decode()) {
    Serial.println (irrecv.decodedIRData.decodedRawData, HEX);
  digitalWrite(10, HIGH); // мигаю светодиодом показываю что дистанционка работает 
    key_code = irrecv.decodedIRData.decodedRawData;

    irrecv.resume(); // Receive the next value

    delay(300); // Wait a bit between retransmissions
     digitalWrite(10, LOW);
    irrecv.enableIRIn(); // Start the receiver
  }
  
  
  
   digitalWrite(7, LOW);    // turn the louvers  off by making the voltage LOW    
   digitalWrite(8, LOW);    // turn the louvers off by making the voltage LOW  
   incomingByte =0;
  
} // end loop

void ProcessCommand (  unsigned long key_code){
  
 //   storeCode(&results);
   Serial.print ( "key_code =" );
   Serial.println (key_code); 
 
 
    if  (key_code == 16642 || key_code == 50) // 2 на пульте
    {
       if (IsTVBoxOn) {
         IsTVBoxOn =false;
         }
       else {
         IsTVBoxOn =true; 
       }
     }   
}

  

   // delay(2000);




//------------------------------------------------------------------------
//------------------------------------------------------------------------
