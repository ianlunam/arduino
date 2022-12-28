/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-save-data-permanently-preferences/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Preferences.h>

Preferences preferences;

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
  bool enabled;
};

void setup() {
  Serial.begin(115200);
  Serial.println();

  struct AlarmEntry alarmEntry;

  bool result = getAlarm("Ians", alarmEntry);
  if (!result) {
    Serial.println("Nah");
    return;
  }

  Serial.print("Name: ");
  Serial.println(alarmEntry.name);
  Serial.print("Hour: ");
  Serial.println(alarmEntry.hour);
  Serial.print("Minute: ");
  Serial.println(alarmEntry.minute);
  Serial.print("Sunday: ");
  Serial.println(alarmEntry.sunday);
  Serial.print("Monday: ");
  Serial.println(alarmEntry.monday);
  Serial.print("Skip Phols: ");
  Serial.println(alarmEntry.skip_phols);
  Serial.print("Enabled: ");
  Serial.println(alarmEntry.enabled);
}

void loop() {
}



bool getAlarm(char* name, AlarmEntry& newAlarm) {
  AlarmEntry thisAlarm;
  for (int x = 0; x < 6; x++) {
    bool result = getAlarm(x, thisAlarm);
    if (result) {
      if (thisAlarm.name == name) {
        Serial.println("GOT IT");
        memcpy(&newAlarm, &thisAlarm, sizeof(thisAlarm));
        return true;
      }
    }
  }
  return false;
}

bool getAlarm(int key, AlarmEntry& newAlarm) {
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
      Serial.print("Got it: name=");
      Serial.println(newAlarm.name);
      alarmStore.end();
      return true;
    }
  }
  alarmStore.end();
  return false;
}
