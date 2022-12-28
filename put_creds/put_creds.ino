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

const char* ssid = "xxxxxx";
const char* password = "xxxxxxx";
const char* data = "xxxxxxxxxxxx";

void setup() {
  Serial.begin(115200);
  Serial.println();

  struct AlarmEntry alarmEntry;
  alarmEntry.name = "Ians";
  alarmEntry.hour = "07";
  alarmEntry.minute = "15";
  alarmEntry.sunday = false;
  alarmEntry.monday = true;
  alarmEntry.tuesday = true;
  alarmEntry.wednesday = true;
  alarmEntry.thursday = true;
  alarmEntry.friday = true;
  alarmEntry.saturday = false;
  alarmEntry.skip_phols = true;
  alarmEntry.enabled = true;

  preferences.begin("alarmStore", false);
  preferences.putBytes("0", &alarmEntry, sizeof(alarmEntry));
  // preferences.putString("apiKey", data); 
  //String pwd = preferences.gettString("password");

  preferences.end();

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
