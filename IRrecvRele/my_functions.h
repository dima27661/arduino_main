//#ifndef MY_FUNCTIONS_H
//#define MY_FUNCTIONS_H

#include <Arduino.h>

struct DateStruct {
  byte day;
  byte month;
  int year;
};
int addTwoInts(int a, int b);
DateStruct logname(char const *date, char *buff);
DateStruct ConvertDate (char const *date);


//#endif
