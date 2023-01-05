#ifndef Alarm_h
#define Alarm_h
#include <Arduino.h>

struct AlarmEntry {
  String name;
  String hour;
  String minute;
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


#define ALARM_ONE alarm1
#define ALARM_TWO alarm2
#define ALARM_THREE alarm3
#define ALARM_FOUR alarm4
#define ALARM_FIVE alarm5
#define ALARM_SIX alarm6


class Alarm {
public:
  Alarm();
  bool init();
  bool setAlarm(int key, AlarmEntry& alarmEntry);
  bool getAlarms(AlarmEntry newEntry[]);
  bool getAlarm(char* name, AlarmEntry& newEntry);
  bool alarming(bool isPhol);
  void turnOff();
  void snooze();
  void turnOn();
  String toString(AlarmEntry& thisEntry);
private:
  bool alarmTriggerNow(bool isPhol);
};

#endif
