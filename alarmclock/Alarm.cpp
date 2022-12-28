#include <Arduino.h>
#include <Preferences.h>

#include "Alarm.h"

int alarmCounter = -1;
AlarmEntry alarms[6];


Alarm::Alarm() {}


void Alarm::init() {
  alarmCounter = -1;

  for (int x = 0; x < 6; x++) {
    AlarmEntry thisAlarm;
    bool happy = getAlarm(x, thisAlarm);
    if (happy) {
      // Serial.print("Name: ");
      // Serial.println(thisAlarm.name);
      // Serial.print("Hour: ");
      // Serial.println(thisAlarm.hour);
      // Serial.print("Minute: ");
      // Serial.println(thisAlarm.minute);
      // Serial.print("Sunday: ");
      // Serial.println(thisAlarm.sunday);
      // Serial.print("Monday: ");
      // Serial.println(thisAlarm.monday);
      // Serial.print("Skip Phols: ");
      // Serial.println(thisAlarm.skip_phols);
      // Serial.print("Enabled: ");
      // Serial.println(thisAlarm.enabled);
      alarms[++alarmCounter] = thisAlarm;
    }
  }
}


bool Alarm::setAlarm(int key, AlarmEntry& alarmEntry) {
  char cstr[3];
  itoa(key, cstr, 2);
  Preferences preferences;
  preferences.begin("alarmStore", false);
  preferences.putBytes(cstr, &alarmEntry, sizeof(alarmEntry));
  preferences.end();
  init();
}


bool Alarm::getAlarm(char* name, AlarmEntry& newAlarm) {
  AlarmEntry thisAlarm;
  for (int x = 0; x < 6; x++) {
    bool result = getAlarm(x, thisAlarm);
    if (result) {
      if (thisAlarm.name == name) {
        memcpy(&newAlarm, &thisAlarm, sizeof(thisAlarm));
        return true;
      }
    }
  }
  return false;
}


bool Alarm::getAlarm(int key, AlarmEntry& newAlarm) {
  Preferences alarmStore;
  alarmStore.begin("alarmStore", true);

  char cstr[3];
  itoa(key, cstr, 2);
  if (alarmStore.isKey(cstr)) {
    int size = alarmStore.getBytesLength(cstr);
    if (size > 0) {
      char* buf[size + 1];
      int result = alarmStore.getBytes(cstr, &buf, size);
      memcpy(&newAlarm, buf, size);
      alarmStore.end();
      return true;
    }
  }
  alarmStore.end();
  return false;
}


bool Alarm::alarming() {
  return false;
}
