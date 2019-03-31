#include "ClockTime.h"

ClockTime::ClockTime()
{
}

ClockTime::ClockTime(uint8_t h, uint8_t m, uint8_t s)
{
  setClockTime(h,m,s);
}

bool ClockTime::isSet()
{
  if(_hour != 0 || _minute != 0 || _second != 0) {return true;}
  return false;
}

void ClockTime::setClockTime(uint8_t h, uint8_t m, uint8_t s)
{
  _hour = h;
  _minute = m;
  _second =s;
}

uint16_t ClockTime::getDeltaMinWith(ClockTime * c )
{
  int16_t dh = (hour() - c->hour()) * 60;
  float d = 0;
  if(dh < 0){dh+=24*60; }
  float val = dh + minute() - c -> minute() + ( second() - c -> second() ) / 60.0;
/*
   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * 100;
   else
       frac = (int(val)- val ) * 100;
   Serial.println(frac,DEC) ;
*/
  return uint16_t(val);
}

uint8_t ClockTime::hour(){return _hour;}
uint8_t ClockTime::minute(){return _minute;}
uint8_t ClockTime::second(){return _second;}

char * ClockTime::getPrintableString()
{
  //hh:mm:ss
  sprintf(_printableString, "%02d:%02d:%02d",hour(), minute(), second());
  return _printableString;
}
