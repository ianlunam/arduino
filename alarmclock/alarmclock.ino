#include <Arduino.h>

#include "Network.h"
#include "Clock.h"
#include "WebContent.h"
#include "Display.h"
#include "Alarm.h"
#include "Pitches.h"


#define SNOOZE_BUTTON 12
#define OFF_BUTTON 13
#define SPEAKER_PIN 27


int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};


Network network;
Clock myClock;
WebContent webContent;
Display oled;
Alarm myAlarms;

time_t lastOffHit = 0;
time_t lastSnoozeHit = 0;
bool snoozeHit = false;
bool offHit = false;

void IRAM_ATTR toggleSnooze() {
  if (lastSnoozeHit < (millis() - 1000)) {
    Serial.println("Toggle snooze button");
    snoozeHit = true;
    lastSnoozeHit = millis();
  }
}


void IRAM_ATTR toggleOff() {
  if (lastOffHit < (millis() - 1000)) {
    Serial.println("Toggle off button");
    offHit = true;
    lastOffHit = millis();
  }
}


void setup() {
  // Start serial
  Serial.begin(115200);
  Serial.println("Starting");

  //Init display
  oled.init();

  // Connect to WiFi
  network.connect();

  // Init clock
  myClock.init();
  oled.setString(0, myClock.getTimeString());
  oled.setString(1, myClock.getDateString());

  // Init web content (phols, weather)
  webContent.init();
  oled.setString(2, webContent.getWeatherString());
  if (webContent.isTodayAHoliday()) {
    oled.setString(3, "P");
  } else {
    oled.setString(3, "N");
  }

  // Reload alarms from memory
  myAlarms.init();

  // Init button interrupts
  pinMode(SNOOZE_BUTTON, INPUT);
  attachInterrupt(SNOOZE_BUTTON, toggleSnooze, RISING);
  pinMode(OFF_BUTTON, INPUT);
  attachInterrupt(OFF_BUTTON, toggleOff, RISING);

  Serial.println("Up and runnning");
}


void loop() {
  delay(100);  // 0.1s

  // Update main screen
  oled.setString(0, myClock.getTimeString());
  oled.setString(1, myClock.getDateString());
  oled.setString(2, webContent.getWeatherString());

  // Update phol flag on screen
  if (webContent.isTodayAHoliday()) {
    oled.setString(3, "P");
  } else {
    oled.setString(3, " ");
  }

  // NTP check run
  myClock.update();

  // Web content refresh
  webContent.update();

  // Check alarm status
  if (myAlarms.alarming(webContent.isTodayAHoliday())) {
    soundBeeper();
  }
  if (snoozeHit) {
    if (myAlarms.alarming(webContent.isTodayAHoliday())) {
      Serial.println("Alarming snooze button noticed");
      myAlarms.snooze();
    } else {
      Serial.println("Not alarming snooze button noticed");
      oled.setString(1, network.ipAddress());
      delay(5000);
    }
    snoozeHit = false;
  }
  if (offHit) {
    if (myAlarms.alarming(webContent.isTodayAHoliday())) {
      Serial.println("Alarming button off noticed");
      myAlarms.turnOff();
    } else {
      Serial.println("Not alarming button off noticed");
      myAlarms.turnOn();
    }
    offHit = false;
  }
}

void soundBeeper() {
  for (;;) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(SPEAKER_PIN, melody[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(SPEAKER_PIN);
      if (snoozeHit or offHit) return;
    }
    delay(1000 / portTICK_PERIOD_MS);
    if (snoozeHit or offHit) return;
  }
}


void tone(byte pin, int freq, int duration) {
  ledcSetup(0, 2000, 8);   // setup beeper
  ledcAttachPin(pin, 0);   // attach beeper
  ledcWriteTone(0, freq);  // play tone
  delay(duration);
  ledcWrite(0, 0);
}


void noTone(byte pin) {
  ledcWrite(0, 0);
}