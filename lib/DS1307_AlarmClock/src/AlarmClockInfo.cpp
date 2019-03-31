#include "AlarmClockInfo.h"

AlarmClockInfo::AlarmClockInfo()
{

}

void AlarmClockInfo::init(uint8_t id, ClockTime c, bool week, bool wkdn)
{
  _id = id;
  _time.setClockTime(c.hour(),c.minute(), c.second());
  _onWeek = week;
  _onWkdn = wkdn;
  _numSnooze = 0;
}

void AlarmClockInfo::init(uint8_t id, uint8_t hour, uint8_t min, uint8_t sec, bool week, bool wkdn)
{
  ClockTime c (hour, min,sec);
  init(id,c,week, wkdn);
}

uint8_t AlarmClockInfo::id(){return _id;}
char * AlarmClockInfo::userInfo(){return _userInfo;}
ClockTime AlarmClockInfo::time(){return _time;}
bool AlarmClockInfo::onWkdn(){return _onWkdn;}
bool AlarmClockInfo::onWeek(){return _onWeek;}
bool AlarmClockInfo::isOn(){return _isOn;}
bool AlarmClockInfo::isOff(){return !_isOn;}
void AlarmClockInfo::setOn()
{
  _isOn = true;
}

void AlarmClockInfo::setOff()
{
  _isOn = false;
}

uint8_t AlarmClockInfo::numSnooze(){return _numSnooze;}
void AlarmClockInfo::setNumSnooze(uint8_t s){_numSnooze = s ;}
