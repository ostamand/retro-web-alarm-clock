#ifndef CLOCKTIME_H
#define CLOCKTIME_H

#include <Arduino.h>

class ClockTime
{
	public:

	ClockTime(uint8_t h, uint8_t m, uint8_t s);
  ClockTime();

	bool 						isSet();
	uint8_t 				hour(); // 0-24
	uint8_t 				minute();  // 0-60
	uint8_t 				second();  // 0-60
	uint16_t 				getDeltaMinWith(ClockTime * c);
	char * 					getPrintableString();
	void 						setClockTime(uint8_t h, uint8_t m, uint8_t s);

	private:

	uint8_t 				_hour,_minute,_second;
	char 						_printableString[9];

};

#endif
