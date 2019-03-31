#include "ClockDateTime.h"

ClockDateTime::ClockDateTime()
{

}

ClockDateTime::ClockDateTime(uint16_t y,uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss )
{
  ClockDateTime();
  init(y,m,d,hh,mm,ss,DayOfTheWeek::MONDAY); // by default Monday if not set by user
}

ClockDateTime::ClockDateTime(uint16_t y,uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss, DayOfTheWeek dd)
{
  ClockDateTime();
  init(y,m,d,hh,mm,ss,dd);
}

void ClockDateTime::init(uint16_t y,uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss, DayOfTheWeek dayWeek)
{
  _year = y;
  _month =m;
  _day = d;
  _dayOfTheWeek = dayWeek;

  setClockTime(hh,mm,ss);
}


DayOfTheWeek ClockDateTime::convertIntToDay(uint8_t d){
  DayOfTheWeek dd;
  switch (d)
  {
    case 1:
    dd = MONDAY;
    break;
    case 2:
    dd = TUESDAY;
    break;
    case 3:
    dd = WEDNESDAY;
    break;
    case 4:
    dd = THURSDAY;
    break;
    case 5:
    dd = FRIDAY;
    break;
    case 6:
    dd = SATURDAY;
    break;
    case 7:
    dd = SUNDAY;
    break;
  }
}

ClockDateTime * ClockDateTime::copyTo(ClockDateTime * dest)
{
  dest -> init(year(),month(),day(),hour(),minute(),second(),dayOfTheWeek());
}

uint16_t ClockDateTime::year(){return _year;}
uint8_t ClockDateTime::month(){return _month;}
uint8_t ClockDateTime::day(){return _day;}
DayOfTheWeek ClockDateTime::dayOfTheWeek(){return _dayOfTheWeek;}

bool ClockDateTime::onWkdn()
{
  if(dayOfTheWeek() == DayOfTheWeek::SATURDAY || dayOfTheWeek() == DayOfTheWeek::SUNDAY)
  {
    return true;
  }
  return false;
};

bool ClockDateTime::onWeek()
{
  if(!onWkdn())
  {
    return true;
  }
  return false;
}

char * ClockDateTime::getPrintableString()
{
  const char * dw ;
  switch (dayOfTheWeek())
  {
     case MONDAY:
      dw = "Monday";
      break;
     case TUESDAY:
      dw = "Tuesday";
      break;
     case WEDNESDAY:
      dw = "Wednesday";
      break;
     case THURSDAY:
      dw = "Thursday";
      break;
     case FRIDAY:
      dw = "Friday";
      break;
     case SATURDAY:
      dw = "Saturday";
      break;
     case SUNDAY:
      dw = "Sunday";
      break;
    }
    sprintf(_printableString, "%s %04d-%02d-%02d %02d:%02d:%02d", dw, year(), month(), day(), hour(), minute(), second() );
    return _printableString;
}
