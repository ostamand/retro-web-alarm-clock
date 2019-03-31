#include "DS1307_AlarmClock.h"
#include "AlarmClockInfo.h"
#include <Wire.h>

DS1307_AlarmClock::DS1307_AlarmClock(uint8_t ds1307_address,
                                     uint8_t snooze_time_min)
{
  DS1307_AlarmClock(ds1307_address, snooze_time_min, 5);  // by default 5 snooze
}

DS1307_AlarmClock::DS1307_AlarmClock(uint8_t ds1307_address,
                                     uint8_t snooze_time_min,
                                     uint8_t num_snooze_max)
{
  _ds1307Address = ds1307_address;
  _snoozeTime = snooze_time_min;
  _numSnoozeMax = num_snooze_max;
  Wire.begin();
}

static uint8_t bcd2bin (uint8_t val)
{
  return val - 6 * (val >> 4);
}

static uint8_t bin2bcd (uint8_t val)
{
  return val + 6 * (val / 10);
}

// writes alarms config to DS1307 RAM
void DS1307_AlarmClock::writeToRAM()
{
  Wire.beginTransmission(_ds1307Address);
  Wire.write(0x08);

  if(_alarmsCount > 0)
  {
    Wire.write(_alarmsCount); // BYTE 1: number of alarms
    int i;
    for(i = 0 ; i< _alarmsCount; i++)
    {
      Wire.write(_alarms[i].id());
      Wire.write(_alarms[i].time().hour());
      Wire.write(_alarms[i].time().minute());
      Wire.write(_alarms[i].time().second());
      if(_alarms[i].onWeek())
      {
        Wire.write(1);
      }
      else
      {
        Wire.write(0);
      }
      if(_alarms[i].onWkdn())
      {
        Wire.write(1);
      }
      else
      {
        Wire.write(0);
      }
    }
  }
  Wire.endTransmission();
}

// reads alarms config from DS1307 RAM
bool DS1307_AlarmClock::readFromRAM()
{
  /*
  56 byte starting at 08h
  BYTE 1:    number of alarms
  BYTE 2:    id
  BYTE 3:    hour
  BYTE 4:    min
  BYTE 5:    sec
  BYTE 6:    week
  BYTE 7:    wkdn
  */

  // get number of alarms in RAM
  Wire.beginTransmission(_ds1307Address);
  Wire.write(0x08); // RAM address
  Wire.endTransmission();
  Wire.requestFrom(_ds1307Address, (uint8_t)1);

  uint8_t numOfAlarms = Wire.read();

  if(numOfAlarms == 0){return false;} // guard stop

  // get alarms
  Wire.beginTransmission(_ds1307Address);
  Wire.write(0x08); // RAM address
  Wire.endTransmission();
  Wire.requestFrom(_ds1307Address, (uint8_t)(1+6*numOfAlarms));
  Wire.read(); // # of alarms in RAM, already known

  // free memory if already allocated, should never be the case
  if(_alarms != NULL){ free(_alarms);}

  _alarms = (AlarmClockInfo *)malloc( sizeof(AlarmClockInfo) * numOfAlarms);
  _alarmsCount = numOfAlarms;

  // define alarms
  int i;
  for(i = 0; i< numOfAlarms; i++)
  {
    uint8_t id = Wire.read();
    uint8_t hh = Wire.read();
    uint8_t mm = Wire.read();
    uint8_t ss = Wire.read();
    uint8_t b = Wire.read();

    bool week = false;
    if(b == 1){ week = true;}
    b = Wire.read();
    bool wkdn = false;
    if(b == 1){ wkdn = true;}
    _alarms[i].init(id,hh,mm,ss,week,wkdn);
  }
  return true;
}

// set DS1307 clock halt
void DS1307_AlarmClock::setClockHalt(bool b)
{
  uint8_t bytes[1];
  readRegister(bytes,0,1);
  Wire.beginTransmission(_ds1307Address);
  Wire.write((byte)0);
  if(b)
  {
    Wire.write(bytes[0] & 0x7F); // on, CH=0
  }
  else
  {
    Wire.write(bytes[0] | 0x80 ); // off, CH=1
  }
  Wire.endTransmission();
}

void DS1307_AlarmClock::readRegister(uint8_t * a, uint8_t address, uint8_t numOfBytes )
{
  Wire.beginTransmission(_ds1307Address);
  Wire.write((byte)address);
  Wire.endTransmission();
  Wire.requestFrom(_ds1307Address, numOfBytes);
  int i;
  for(i=0; i< numOfBytes; i++)
  {
    a[i] = Wire.read();
  }
}

bool DS1307_AlarmClock::isClockRunning()
{
  uint8_t bytes[1];
  readRegister(bytes,0,1);
  return !(bytes[0] >> 7);
}

ClockDateTime * DS1307_AlarmClock::ds1307_getTime()
{
  Wire.beginTransmission(_ds1307Address);
  Wire.write((byte)0);
  Wire.endTransmission();
  Wire.requestFrom(_ds1307Address, (uint8_t)7);
  uint8_t ss = bcd2bin(Wire.read() & 0x7F);
  uint8_t mm = bcd2bin(Wire.read());
  uint8_t hh = bcd2bin(Wire.read());
  DayOfTheWeek dw = static_cast<DayOfTheWeek>(bcd2bin(Wire.read()));
  uint8_t d = bcd2bin(Wire.read());
  uint8_t m = bcd2bin(Wire.read());
  uint16_t y = bcd2bin(Wire.read()) + 2000;
  _currentClockDateTime.init(y,m,d,hh,mm,ss,dw);
  return &_currentClockDateTime;
}

ClockDateTime * DS1307_AlarmClock::getCurrentTime()
{
  return &_currentClockDateTime;
}

void DS1307_AlarmClock::setCurrentTime(ClockDateTime t)
{
    Wire.beginTransmission(_ds1307Address);
    Wire.write((byte)0); // start at location 0
    Wire.write(bin2bcd(t.second()));
    Wire.write(bin2bcd(t.minute()));
    Wire.write(bin2bcd(t.hour()));
    Wire.write(bin2bcd(t.dayOfTheWeek()));
    Wire.write(bin2bcd(t.day()));
    Wire.write(bin2bcd(t.month()));
    Wire.write(bin2bcd(t.year() - 2000));
    Wire.endTransmission();
}

void DS1307_AlarmClock::run()
{
  int i;
	ClockDateTime * c = ds1307_getTime();

	if(c != NULL)
	{
		for(i=0; i<_alarmsCount; i++)
		{
			if( _alarms[i].isOn() ) // Snoozed alarm currently running
			{
        uint16_t delta = c -> getDeltaMinWith(&_lastAlarm);
				if(  delta >= _snoozeTime)
				{
          c -> copyTo(&_lastAlarm);
          _alarms[i].setNumSnooze(_alarms[i].numSnooze()+1);
          if(_alarms[i].numSnooze() > _numSnoozeMax )
          {
            _alarms[i].setOff();  // num of snooze max reached
          }
          else
          {
            if(_alarmCallback != NULL)
            {
              _alarmCallback(&_alarms[i]); // snoozed alarm
            }
          }
				}
			}
			else
			{
				// Check day of the week first. Weekend or week
				if( (_alarms[i].onWkdn() && c -> onWkdn()) || (_alarms[i].onWeek() && c -> onWeek()) )
				{
					// Check time hh:mm
					if( (_alarms[i].time().hour() == c -> hour()) && (_alarms[i].time().minute() == c -> minute()) && (_alarms[i].time().second() == c -> second()) )
					{
						c -> copyTo(&_lastAlarm);
						_alarms[i].setOn();
						if(_alarmCallback != NULL)
						{
							_alarmCallback(&_alarms[i]); // alarm first time
						}
					}
				}
			}
		}
	}
}

void DS1307_AlarmClock::onAlarm(AlarmClockCallback cb)
{
	_alarmCallback =cb;
}

// add or update alarm configurations
bool DS1307_AlarmClock::addOrUpdateAlarm(uint8_t id, uint8_t hour, uint8_t min, uint8_t sec, bool week, bool wkdn)
{
  AlarmClockInfo * at;
  int i;
  bool found = false;

  // find by id
  for(i=0; i<_alarmsCount; i++)
  {
    if(_alarms[i].id() == id)
    {
      at = &_alarms[i];
      found = true;
      break;
    }
  }

  if(!found)
  {
    _alarmsCount++;
    if(_alarms == NULL)
    {
      _alarms = (AlarmClockInfo *)malloc(_alarmsCount * sizeof(AlarmClockInfo) );
    }
    else
    {
      _alarms = (AlarmClockInfo *)realloc(_alarms, _alarmsCount * sizeof(AlarmClockInfo));
    }
    at = &_alarms[_alarmsCount-1];
  }

  // update alarm
  at -> init(id,hour,min,sec,week,wkdn);

  // alarm currently off
  at -> setOff();

  // save new alarm configs to DS1307 RAM
  writeToRAM();

  return true;
}

uint8_t DS1307_AlarmClock::getNumSnoozeMax()
{
  return _numSnoozeMax;
}

AlarmClockInfo * DS1307_AlarmClock::getAlarmWithID(uint8_t id)
{
  int i;
  for(i=0; i<_alarmsCount; i++)
  {
    if(_alarms[i].id() == id )
    {
      return &_alarms[i];
    }
  }
}

uint8_t DS1307_AlarmClock::getNumOfAlarms()
{
  return _alarmsCount;
}

AlarmClockInfo * DS1307_AlarmClock::getAlarms()
{
	return _alarms;
}
