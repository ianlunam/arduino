#ifndef Clock_h
#define Clock_h
#include <Arduino.h>

class Clock {
public:
  Clock();
  void init();
  void update();
  void connect(const char* ssid, const char* wifipw);
  void setTime();
  String getTimeString();
  String getDateString();
  String getYearString();
};

#endif
