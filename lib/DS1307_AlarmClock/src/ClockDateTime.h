#ifndef CLOCKDATETIME_H
#define CLOCKDATETIME_H

#include <Arduino.h>
#include "ClockTime.h"

enum DayOfTheWeek { MONDAY = 1, TUESDAY = 2, WEDNESDAY = 3, THURSDAY = 4, FRIDAY = 5 , SATURDAY = 6, SUNDAY = 7};

class ClockDateTime: public ClockTime
{
	public:

  ClockDateTime();
	ClockDateTime(uint16_t y,uint8_t m, uint8_t d,
								uint8_t hh, uint8_t mm, uint8_t ss );
	ClockDateTime(uint16_t y,uint8_t m, uint8_t d, uint8_t hh,
							  uint8_t mm, uint8_t ss, DayOfTheWeek dd);

	void init(uint16_t y,uint8_t m, uint8_t d, uint8_t hh, uint8_t mm,
					  uint8_t ss, DayOfTheWeek dayWeek);

	static DayOfTheWeek 	convertIntToDay(uint8_t d);

	ClockDateTime * 				copyTo(ClockDateTime * dest);
	uint16_t 								year();
	uint8_t 								month();
	uint8_t 								day();
	bool 										onWkdn();
	bool 										onWeek();
	DayOfTheWeek 						dayOfTheWeek();
	char * 									getPrintableString();

	private:

	uint16_t 	 							_year;
	uint8_t 								_month, _day;
	DayOfTheWeek 						_dayOfTheWeek;
	char 										_printableString[30];
};

#endif
