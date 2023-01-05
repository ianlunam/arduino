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

// const char* ssid = "xxxxx";
// const char* password = "xxxxx";
// const char* data = "xxxxx";

void setup() {
  Serial.begin(115200);
  Serial.println();

  preferences.begin("alarmStore", false);

  char alarmList[][20] = {"", "", "", "", "", ""};

  AlarmEntry alarmEntry;

  // alarmEntry.name = "Ian";
  strcpy(alarmEntry.name, "Ian");
  alarmEntry.hour = 6;
  alarmEntry.minute = 30;
  alarmEntry.sunday = false;
  alarmEntry.monday = true;
  alarmEntry.tuesday = true;
  alarmEntry.wednesday = true;
  alarmEntry.thursday = true;
  alarmEntry.friday = true;
  alarmEntry.saturday = false;
  alarmEntry.skip_phols = true;
  alarmEntry.skip_phols = false;
  alarmEntry.enabled = true;

  // alarmList[0] = alarmEntry.name;
  strcpy(alarmList[0], alarmEntry.name);
  preferences.putBytes(alarmEntry.name, &alarmEntry, sizeof(alarmEntry));

  // alarmEntry.name = "Janet";
  strcpy(alarmEntry.name, "Janet");
  alarmEntry.hour = 6;
  alarmEntry.minute = 0;
  alarmEntry.sunday = false;
  alarmEntry.monday = true;
  alarmEntry.tuesday = true;
  alarmEntry.wednesday = true;
  alarmEntry.thursday = true;
  alarmEntry.friday = true;
  alarmEntry.saturday = false;
  alarmEntry.skip_phols = true;
  alarmEntry.skip_phols = false;
  alarmEntry.enabled = true;

  // alarmList[1] = alarmEntry.name;
  strcpy(alarmList[1], alarmEntry.name);
  preferences.putBytes(alarmEntry.name, &alarmEntry, sizeof(alarmEntry));

  preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
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

  // preferences.begin("wifiCreds", false);
  // preferences.putString("ssid", ssid);
  // preferences.putString("password", password);
  // preferences.end();

  // preferences.begin("holidayApi", false);
  // preferences.putString("apiKey", data);
  // preferences.end();
}

void loop() {

}
