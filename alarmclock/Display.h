#ifndef Display_h
#define Display_h
#include <Arduino.h>

class Display {
public:
  Display();
  void init();
  void updateDisplay();
  void setString(int zone, String value);
};

#endif
