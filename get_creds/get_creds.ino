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


void setup() {
  Serial.begin(115200);
  Serial.println();

  preferences.begin("wifiCreds", false);
  preferences.putString("ssid", "DontEvenThinkAboutIt");
  preferences.putString("password", "w0bblel0");
  preferences.end();
}

void loop() {
}
