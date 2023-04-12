#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

#include "Alarm.h"

#define ALARM_OFF 0
#define ALARM_ON 1
#define ALARM_SNOOZE 2


int alarmState = ALARM_OFF;

char alarmList[][20] = { "", "", "", "", "", "" };
time_t alarmedLast[6];
time_t snoozeTime = 0;
time_t lastAlarmCheck = 0;
time_t alarmStarted = 0;


Alarm::Alarm() {}


void Alarm::getAlarmList() {
  Preferences preferences;

  preferences.begin("alarmStore", false);
  if (preferences.isKey("alarms")) {
    int size = preferences.getBytesLength("alarms");
    if (size > 0) {
      char* buf[size + 1];
      int result = preferences.getBytes("alarms", &buf, size);
      memcpy(&alarmList, buf, size);
      preferences.end();
      return;
    }
  }
  for (int x = 0; x < 6; x++) {
    strcpy(alarmList[x], "");
  }
  preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
  preferences.end();
  return;
}


bool Alarm::getAlarm(char* name, AlarmEntry& newAlarm) {
  getAlarmList();

  bool inList = false;
  for (int x = 0; x < 6; x++) {
    if (strcmp(alarmList[x], name) == 0) {
      inList = true;
    }
  }

  if (!inList) {
    return false;
  }

  Preferences alarmStore;
  alarmStore.begin("alarmStore", true);

  if (alarmStore.isKey(name)) {
    int size = alarmStore.getBytesLength(name);
    if (size > 0) {
      char* buf[size + 1];
      int result = alarmStore.getBytes(name, &buf, size);
      memcpy(&newAlarm, buf, size);
      alarmStore.end();
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
    if (now > (alarmStarted + (60 * 7))) {
      Serial.println("Snooze over");
      alarmState = ALARM_ON;
      alarmStarted = now;
      return true;
    }
    return false;
  } else {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return false;
    }
    time_t now = mktime(&timeinfo);
    if (alarmTriggerNow(isPhol)) {
      alarmState = ALARM_ON;
      alarmStarted = now;
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


  getAlarmList();

  for (int x = 0; x < 6; x++) {
    if (strlen(alarmList[x]) == 0) {
      continue;
    }
    // Serial.print("Checking alarm ");
    // Serial.println(alarmList[x]);
    AlarmEntry nextAlarm;
    if (getAlarm(alarmList[x], nextAlarm)) {
      // Serial.println(toString(nextAlarm));

      // Skip the alarm if ...
      if (!nextAlarm.enabled) continue;
      if (isPhol and nextAlarm.skip_phols) continue;

      // Skip if today isn't enabled
      if (currentTm.tm_wday == 0 and !nextAlarm.sunday) continue;
      if (currentTm.tm_wday == 1 and !nextAlarm.monday) continue;
      if (currentTm.tm_wday == 2 and !nextAlarm.tuesday) continue;
      if (currentTm.tm_wday == 3 and !nextAlarm.wednesday) continue;
      if (currentTm.tm_wday == 4 and !nextAlarm.thursday) continue;
      if (currentTm.tm_wday == 5 and !nextAlarm.friday) continue;
      if (currentTm.tm_wday == 6 and !nextAlarm.saturday) continue;

      struct tm t = { 0 };
      t.tm_year = currentTm.tm_year;  // Construct tm as per today for alarm time at zero seconds
      t.tm_mon = currentTm.tm_mon;
      t.tm_mday = currentTm.tm_mday;
      t.tm_hour = nextAlarm.hour - currentTm.tm_isdst;
      t.tm_min = nextAlarm.minute;
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
        if (nextAlarm.once) {
          nextAlarm.enabled = false;
          Preferences preferences;
          preferences.begin("alarmStore", false);
          preferences.putBytes(nextAlarm.name, &nextAlarm, sizeof(nextAlarm));
          preferences.end();
        }
        return true;
      }
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

bool Alarm::isSnoozed() {
  if (alarmState == ALARM_SNOOZE) {
    return true;
  } else {
    return false;
  }
}

bool Alarm::isOn() {
  if (alarmState == ALARM_ON) {
    return true;
  } else {
    return false;
  }
}