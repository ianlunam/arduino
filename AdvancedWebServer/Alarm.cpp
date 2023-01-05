#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

#include "Alarm.h"

#define ALARM_OFF 0
#define ALARM_ON 1
#define ALARM_SNOOZE 2


int alarmState = ALARM_OFF;

int alarmCounter = -1;
AlarmEntry alarms[600];
time_t alarmedLast[6];
time_t snoozeTime = 0;
time_t lastAlarmCheck = 0;


Alarm::Alarm() {}


bool Alarm::init() {
  AlarmEntry iansAlarm;
  bool happy = getAlarm("Ian", iansAlarm);
  Serial.print("getAlarms hapy: ");
  Serial.println(happy);
  Serial.flush();

  Serial.println(toString(iansAlarm));
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
  AlarmEntry storedAlarms[6];
  bool result = getAlarms(storedAlarms);
  if (result) {
    for (int i = 0; i < 6; i++) {
      if (storedAlarms[i].name == name) {
        memcpy(&newAlarm, &storedAlarms[i], sizeof(storedAlarms[i]));
        return true;
      }
    }
  }
  return false;
}


bool Alarm::getAlarms(AlarmEntry newAlarms[600]) {
  Preferences alarmStore;
  alarmStore.begin("alarmStore", true);
  Serial.println("Opened alarm store");

  if (alarmStore.isKey("alarms")) {
    Serial.println("Getting alarm preference");
    int size = alarmStore.getBytesLength("alarms");
    Serial.print("Size: ");
    Serial.println(size);
    if (size > 0) {
      char* buf[size + 1];
      int result = alarmStore.getBytes("alarms", &buf, size);
      Serial.println("Got alarms. Copying");
      memcpy(&newAlarms, buf, size+1);
      alarmStore.end();
      Serial.println("Returning alarms");
      return true;
    }
  }
  alarmStore.end();
  return false;
}


bool Alarm::alarming(bool isPhol) {
  if (alarmState == ALARM_ON) {
    return true;
  } else if (alarmState == ALARM_SNOOZE) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return false;
    }
    time_t now = mktime(&timeinfo);
    if (now > (snoozeTime + (60 * 7))) {
      // if (now > (snoozeTime + 20)) {
      alarmState = ALARM_ON;
      return true;
    }
    return false;
  } else {
    if (alarmTriggerNow(isPhol)) {
      alarmState = ALARM_ON;
      return true;
    }
    return false;
  }
}


bool Alarm::alarmTriggerNow(bool isPhol) {
  struct tm currentTm;
  if (!getLocalTime(&currentTm)) {
    return false;
  }
  if (currentTm.tm_sec > 10) return false;  // Only check if in the first 10s of the minute

  time_t currentTime = mktime(&currentTm);
  if (lastAlarmCheck > (currentTime - 20)) return false;  // Only check if we last checked more than 20s ago (ie: greater than 10s)
  lastAlarmCheck = currentTime;

  currentTm.tm_sec = 0;  // reset to first second of minute to make comparison easier
  currentTime = mktime(&currentTm);

  for (int x = 0; x < 6; x++) {
    AlarmEntry nextAlarm = alarms[x];
      Serial.println(toString(nextAlarm));

      // Skip the alarm if ...
      if (!nextAlarm.enabled) continue;
      if (isPhol and nextAlarm.skip_phols) continue;

      // Skip if today isn't enabled
      if (currentTm.tm_wday == 0 and !nextAlarm.sunday) continue;
      if (currentTm.tm_wday == 1 and !nextAlarm.monday) continue;
      if (currentTm.tm_wday == 2 and !nextAlarm.tuesday) continue;
      if (currentTm.tm_wday == 3 and !nextAlarm.wednesday) continue;
      if (currentTm.tm_wday == 4 and !nextAlarm.thursday) continue;
      if (currentTm.tm_wday == 6 and !nextAlarm.friday) continue;
      if (currentTm.tm_wday == 7 and !nextAlarm.saturday) continue;

      struct tm t = { 0 };
      t.tm_year = currentTm.tm_year;  // Construct tm as per today for alarm time at zero seconds
      t.tm_mon = currentTm.tm_mon;
      t.tm_mday = currentTm.tm_mday;
      t.tm_hour = nextAlarm.hour.toInt() - currentTm.tm_isdst;
      t.tm_min = nextAlarm.minute.toInt();
      t.tm_sec = 0;
      time_t alarmTime = mktime(&t);  // convert to seconds
      Serial.print("Now: ");
      Serial.print(currentTime);
      Serial.print(" Alarm: ");
      Serial.print(alarmTime);
      Serial.print(" Lasttime: ");
      Serial.println(alarmedLast[x]);
      if (alarmTime == currentTime && alarmedLast[x] < (currentTime - 20)) {  // Using same 20s as above to debounce
        alarmedLast[x] = currentTime;
        return true;
      }
  }
  return false;
}

void Alarm::turnOff() {
  Serial.println("Alarm off button");
  alarmState = ALARM_OFF;
}

void Alarm::turnOn() {
  Serial.println("Alarm on button");
  alarmState = ALARM_ON;
}


void Alarm::snooze() {
  Serial.println("Alarm snooze button");
  alarmState = ALARM_SNOOZE;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  snoozeTime = mktime(&timeinfo);
}

String Alarm::toString(AlarmEntry& thisAlarm) {
  String output = "Name: ";
  output.concat(thisAlarm.name);
  output.concat(" ");
  output.concat(thisAlarm.hour);
  output.concat(":");
  output.concat(thisAlarm.minute);
  output.concat(" ");
  if (thisAlarm.sunday) {
    output.concat("S");
  } else {
    output.concat(".");
  }
  if (thisAlarm.monday) {
    output.concat("M");
  } else {
    output.concat(".");
  }
  if (thisAlarm.tuesday) {
    output.concat("T");
  } else {
    output.concat(".");
  }
  if (thisAlarm.wednesday) {
    output.concat("W");
  } else {
    output.concat(".");
  }
  if (thisAlarm.thursday) {
    output.concat("T");
  } else {
    output.concat(".");
  }
  if (thisAlarm.friday) {
    output.concat("F");
  } else {
    output.concat(".");
  }
  if (thisAlarm.saturday) {
    output.concat("S");
  } else {
    output.concat(".");
  }

  if (thisAlarm.skip_phols) {
    output.concat("P");
  } else {
    output.concat("N");
  }

  if (thisAlarm.once) {
    output.concat("O");
  } else {
    output.concat("N");
  }

  if (thisAlarm.enabled) {
    output.concat("Y");
  } else {
    output.concat("N");
  }

  return output;
}
