#ifndef Alarm_h
#define Alarm_h
#include <Arduino.h>

struct AlarmEntry {
  char name[20];
  int hour;
  int minute;
  bool sunday;
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saturday;
  bool skip_phols;
  bool once;
  bool enabled;
};

class Alarm {
public:
  Alarm();
  bool getAlarm(char* name, AlarmEntry& newEntry);
  bool alarming(bool isPhol);
  void turnOff();
  void snooze();
  void turnOn();
  String toString(AlarmEntry& thisEntry);
  void getAlarmList();
  bool isSnoozed();
  bool isOn();
private:
  bool alarmTriggerNow(bool isPhol);
};

#endif
