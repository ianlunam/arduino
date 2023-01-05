#ifndef Configurator_h
#define Configurator_h

#include <Arduino.h>
#include "Alarm.h"

class Configurator {
public:
  static String editPage(AlarmEntry &alarmEntry);
  static String newPage();
private:
  static String pageTop;
  static String pageMiddle;
  static String pageBottom;
};

#endif
