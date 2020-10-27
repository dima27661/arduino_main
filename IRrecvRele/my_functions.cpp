#include "my_functions.h"
int addTwoInts(int a, int b)
{
  return a + b;
}
DateStruct logname(char const *date, char *buff) { 
    int month, day, year;
        DateStruct res;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d.txt", year, month, day);
  Serial.println(" logname.year");  
  Serial.print(year);  
    
    res.month = month;
    res.year = year-2000;
    res.day = day;
    return res;
    
}

DateStruct ConvertDate (char const *date){ 
    int month, day, year;
    char buff[16];
    DateStruct res;
   
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d",  buff,&day, &year);
    res.month = (strstr(month_names, buff)-month_names)/3+1;
    res.year = year-2000;
    res.day = day;
    return res;
    //sprintf(buff, "%d%02d%02d.txt", year, month, day);
}
