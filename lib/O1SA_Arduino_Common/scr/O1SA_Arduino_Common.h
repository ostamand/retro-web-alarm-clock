#ifndef O1SA_ARDUINO_COMMON_H
#define O1SA_ARDUINO_COMMON_H

#include <Arduino.h>

class Common
{

public:

  static char **        splitBy(const char * str, const char * sep,
                                uint8_t * num_strings);
  static char *         removeWhiteSpaces(const char * str);

};

#endif
