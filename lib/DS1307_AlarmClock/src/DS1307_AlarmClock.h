#ifndef DS1307_ALARMCLOCK_H
#define DS1307_ALARMCLOCK_H

#include <Arduino.h>
#include "ClockDateTime.h"

class ClockDateTime;
class AlarmClockInfo;

typedef void (*AlarmClockCallback)(AlarmClockInfo *data);

class DS1307_AlarmClock
{

	public:

	DS1307_AlarmClock(uint8_t ds1307_address, uint8_t snooze_time_min);
	DS1307_AlarmClock(uint8_t ds1307_address, uint8_t snooze_time_min,
										uint8_t num_snooze_max);

	void 										run();
	bool 										addOrUpdateAlarm(uint8_t id, uint8_t hour, uint8_t min,
																 			 		 uint8_t sec, bool week, bool wkdn);
	bool 							  		readFromRAM();
	void 										writeToRAM();
	void 										onAlarm(AlarmClockCallback cb);
	ClockDateTime * 				getCurrentTime();
	ClockDateTime * 				ds1307_getTime();
	void 										setCurrentTime(ClockDateTime t);
	void 										setClockHalt(bool b);
	bool 										isClockRunning();
	uint8_t 								getNumSnoozeMax();
	AlarmClockInfo * 				getAlarmWithID(uint8_t id);
	uint8_t 								getNumOfAlarms();
	AlarmClockInfo *					getAlarms();

	private:

	ClockDateTime 						_lastAlarm ;
	AlarmClockCallback					_alarmCallback 		=	NULL;
	AlarmClockInfo * 					_alarms 					= NULL;
	uint8_t									_alarmsCount			=	0;
	uint8_t									_ds1307Address;
	uint8_t									_snoozeTime;			// snooze time in min
	uint8_t									_numSnoozeMax;
	ClockDateTime						_currentClockDateTime;

	void 										readRegister(uint8_t * a, uint8_t address,
																			 uint8_t numOfBytes );
};


#endif
