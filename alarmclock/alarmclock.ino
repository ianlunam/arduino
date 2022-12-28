#include "Network.h"
#include "Clock.h"
#include "WebContent.h"
#include "Display.h"
#include "Alarm.h"


#define SNOOZE_BUTTON 12
#define OFF_BUTTON 2


Network network;
Clock myClock;
WebContent webContent;
Display oled;
Alarm myAlarms;

bool snooze_pressed = false;
bool off_pressed = false;


void IRAM_ATTR toggleSnooze()
{
  snooze_pressed = true;
}

void IRAM_ATTR toggleOff()
{
  off_pressed = true;
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
    oled.setString(3, " ");
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
  vTaskDelay(100);  // 0.1s

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

  // Chark alarm status
  if (myAlarms.alarming()) {
    // Get jiggy wid it
  } else {
    if (snooze_pressed) {
      oled.setString(1, network.ipAddress());
      vTaskDelay(5000);
      snooze_pressed = false;
    }
    if (off_pressed) {
      oled.setString(1, "Whatevs");
      vTaskDelay(5000);
      off_pressed = false;
    }
  }
}
