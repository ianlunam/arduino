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
  bool once;
  bool enabled;
};


void setup() {
  Serial.begin(115200);
  Serial.println();

  AlarmEntry alarmEntries[6];
  AlarmEntry alarmEntry;

  Preferences alarmStore;
  alarmStore.begin("alarmStore", true);
  // int size = alarmStore.getBytesLength("alarms");
  // char* buf[size + 1];
  // int result = alarmStore.getBytes("alarms", &buf, size);
  // memcpy(&alarmEntries, buf, size);
  preferences.remove("0");
  preferences.remove("1");
  preferences.remove("alarms");
  preferences.clear();
  alarmStore.end();

  alarmEntry = alarmEntries[2];

  // Serial.print("Name: ");
  // Serial.println(alarmEntry.name);
  // Serial.print("Hour: ");
  // Serial.println(alarmEntry.hour);
  // Serial.print("Minute: ");
  // Serial.println(alarmEntry.minute);
  // Serial.print("Sunday: ");
  // Serial.println(alarmEntry.sunday);
  // Serial.print("Monday: ");
  // Serial.println(alarmEntry.monday);
  // Serial.print("Skip Phols: ");
  // Serial.println(alarmEntry.skip_phols);
  // Serial.print("Enabled: ");
  // Serial.println(alarmEntry.enabled);
}

void loop() {
}
