#ifndef ALARMCLOCKINFO_H
#define ALARMCLOCKINFO_H

#include <Arduino.h>

#include "ClockTime.h"
#include "ClockDateTime.h"

class AlarmClockInfo
{
	public:

	AlarmClockInfo(uint8_t id, ClockTime c, bool week, bool wkdn);
  AlarmClockInfo();

	void 							init(uint8_t id, ClockTime c, bool week, bool wkdn);
	void 							init(uint8_t id, uint8_t hour, uint8_t min,
												 uint8_t sec, bool week, bool wkdn);

	uint8_t 					id();
	char * 						userInfo();
	ClockTime 				time();
	bool 							onWkdn();
	bool 							onWeek();
	uint8_t 					numSnooze();
	void 							setNumSnooze(uint8_t s);
	bool 							isOn();
	bool 							isOff();
	void 							setOn();
	void 							setOff();

	private:

	char * 						_userInfo;
	uint8_t 					_id;								// alarm id 1..N
	ClockTime 				_time;							// alarm time
	bool 						  _onWkdn=false;			// is alarm on weekend
	bool						  _onWeek=false;			// is alarm on week
	bool							_isOn=false;				// is alarm currently running flag
	uint8_t 					_numSnooze;

};

#endif
