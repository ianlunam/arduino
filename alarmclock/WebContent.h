#ifndef WebContent_h
#define WebContent_h
#include <Arduino.h>

#include "Clock.h"

class WebContent {
public:
  WebContent();
  void init();
  void update();
  String getWeatherString();
  bool isTodayAHoliday();
  String convertCondition(String current);
private:
  void weather();
  void holiday();
  bool isHoliday;
};

#endif