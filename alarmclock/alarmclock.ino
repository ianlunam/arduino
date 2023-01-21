#include <Arduino.h>

// Different machines :)
#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include "Network.h"
#include "Clock.h"
#include "Configurator.h"
#include "WebContent.h"
#include "Display.h"
#include "Alarm.h"
#include "Pitches.h"


#define SNOOZE_BUTTON 32
#define OFF_BUTTON 33
#define SPEAKER_PIN 27


int melody[] = {
  NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5
};

int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4, 4
};


Network network;
Clock myClock;
WebContent webContent;
Display oled;
Alarm myAlarms;
Configurator configurator;

AsyncWebServer server(80);
const char *PARAM_MESSAGE = "message";

time_t lastOffHit = 0;
time_t lastSnoozeHit = 0;
bool snoozeHit = false;
bool offHit = false;
bool alarmsNeedUpdate = false;

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


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


String alarmToTableRow(AlarmEntry &thisAlarm) {
  String output = "<tr><td>";
  output.concat(thisAlarm.name);
  output.concat("</td><td>");
  if (thisAlarm.hour < 10) output.concat("0");
  output.concat(thisAlarm.hour);
  output.concat(":");
  if (thisAlarm.minute < 10) output.concat("0");
  output.concat(thisAlarm.minute);
  output.concat("</td><td>");
  if (thisAlarm.sunday) {
    output.concat("S");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.monday) {
    output.concat("M");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.tuesday) {
    output.concat("T");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.wednesday) {
    output.concat("W");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.thursday) {
    output.concat("T");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.friday) {
    output.concat("F");
  } else {
    output.concat(".");
  }
  output.concat("</td><td>");
  if (thisAlarm.saturday) {
    output.concat("S");
  } else {
    output.concat(".");
  }
  output.concat("</td><td style=\"text-align:center\">");
  if (thisAlarm.skip_phols) {
    output.concat("Y");
  } else {
    output.concat("N");
  }
  output.concat("</td><td style=\"text-align:center\">");
  if (thisAlarm.once) {
    output.concat("Y");
  } else {
    output.concat("N");
  }
  output.concat("</td><td style=\"text-align:center\">");
  if (thisAlarm.enabled) {
    output.concat("Y");
  } else {
    output.concat("N");
  }
  output.concat("</td><td><a href=\"/edit?name=");
  output.concat(thisAlarm.name);
  output.concat("\">Edit</a> or <a href=\"/delete?name=");
  output.concat(thisAlarm.name);
  output.concat("\">Delete</a></td></tr>\n");
  return output;
}


bool getAlarm(char *name, AlarmEntry &newAlarm) {

  Preferences alarmStore;
  alarmStore.begin("alarmStore", true);

  if (alarmStore.isKey(name)) {
    int size = alarmStore.getBytesLength(name);
    if (size > 0) {
      char *buf[size + 1];
      int result = alarmStore.getBytes(name, &buf, size);
      memcpy(&newAlarm, buf, size);
      alarmStore.end();
      return true;
    }
  }
  alarmStore.end();
  return false;
}


void handleEdit(AsyncWebServerRequest *request) {
  if (!request->hasArg("name")) {
    request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
    return;
  }

  AlarmEntry alarmEntry;
  if (getAlarm((char *)request->getParam("name")->value().c_str(), alarmEntry)) {
    request->send(200, "text/html", configurator.editPage(alarmEntry));
  } else {
    request->send(404, "text/plain", request->getParam("name")->value() + " doesn't exist.");
  }
}


void handleNew(AsyncWebServerRequest *request) {
  request->send(200, "text/html", configurator.newPage());
}


void handleSave(AsyncWebServerRequest *request) {
  if (!request->hasArg("name")) {
    request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
    return;
  }

  AlarmEntry alarmEntry = {"", 0, 0, false, false, false, false, false, false, false, false, false, false};
  Preferences preferences;
  strcpy(alarmEntry.name, request->getParam("name")->value().c_str());
  alarmEntry.hour = request->getParam("hour")->value().toInt();
  alarmEntry.minute = request->getParam("minute")->value().toInt();
  if (request->hasArg("sunday")) { alarmEntry.sunday = true; }
  if (request->hasArg("monday")) { alarmEntry.monday = true; }
  if (request->hasArg("tuesday")) { alarmEntry.tuesday = true; }
  if (request->hasArg("wednesday")) { alarmEntry.wednesday = true; }
  if (request->hasArg("thursday")) { alarmEntry.thursday = true; }
  if (request->hasArg("friday")) { alarmEntry.friday = true; }
  if (request->hasArg("saturday")) { alarmEntry.saturday = true; }
  if (request->hasArg("skip_phols")) { alarmEntry.skip_phols = true; }
  if (request->hasArg("once")) { alarmEntry.once = true; }
  if (request->hasArg("enabled")) { alarmEntry.enabled = true; }
  preferences.begin("alarmStore", false);
  preferences.putBytes(alarmEntry.name, &alarmEntry, sizeof(alarmEntry));

  char alarmList[][20] = { "", "", "", "", "", "" };
  if (preferences.isKey("alarms")) {
    int size = preferences.getBytesLength("alarms");
    if (size > 0) {
      char *buf[size + 1];
      int result = preferences.getBytes("alarms", &buf, size);
      memcpy(&alarmList, buf, size);
    }
  } else {
    for (int x = 0; x < 6; x++) {
      strcpy(alarmList[x], "");
    }
  }
  bool found = false;
  int freePos = -1;
  for (int x = 0; x < 6; x++) {
    if (strcmp(alarmList[x], request->getParam("name")->value().c_str()) == 0) {
      found = true;
    } else if (strlen(alarmList[x]) == 0 && freePos == -1) {
      freePos = x;      
    }
  }
  if (!found) {
    if (freePos > -1) {
      strcpy(alarmList[freePos], request->getParam("name")->value().c_str());
    }
  }
  preferences.putBytes("alarms", &alarmList, sizeof(alarmList));

  preferences.end();

  request->redirect("/");
}


void handleDelete(AsyncWebServerRequest *request) {
  if (!request->hasArg("name")) {
    request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
    return;
  }

  char alarmList[][20] = { "", "", "", "", "", "" };
  Preferences preferences;

  preferences.begin("alarmStore", false);
  if (preferences.isKey("alarms")) {
    int size = preferences.getBytesLength("alarms");
    if (size > 0) {
      char *buf[size + 1];
      int result = preferences.getBytes("alarms", &buf, size);
      memcpy(&alarmList, buf, size);
    }
  } else {
    for (int x = 0; x < 6; x++) {
      strcpy(alarmList[x], "");
    }
  }
  for (int x = 0; x < 6; x++) {
    if (strcmp(alarmList[x], request->getParam("name")->value().c_str()) == 0) {
      strcpy(alarmList[x], "");
    }
  }
  preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
  preferences.remove(request->getParam("name")->value().c_str());
  preferences.end();

  request->redirect("/");
}


void handleRoot(AsyncWebServerRequest *request) {
  char alarmList[][20] = { "", "", "", "", "", "" };
  Preferences preferences;

  preferences.begin("alarmStore", false);
  if (preferences.isKey("alarms")) {
    int size = preferences.getBytesLength("alarms");
    if (size > 0) {
      char *buf[size + 1];
      int result = preferences.getBytes("alarms", &buf, size);
      memcpy(&alarmList, buf, size);
    }
  } else {
    for (int x = 0; x < 6; x++) {
      strcpy(alarmList[x], "");
    }
    preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
  }
  preferences.end();

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE html><html><head><title>Current Alarms</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } table, th, td { border: 1px solid black; padding: 5px;}</style></head><body>\n");
  response->print("<h1>Hello from the Alarm Clock!</h1>\n");
  response->print("<p>These are the current alarms set.</p>\n");
  response->print("<table>\n");
  response->print("<tr><th>Name</th><th>Time</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>Skip Holidays</th><th>Once Only</th><th>Enabled</th><th>Change</th></tr>\n");
  for (int x = 0; x < 6; x++) {
    AlarmEntry myAlarm;
    if (getAlarm(alarmList[x], myAlarm)) {
      response->print(alarmToTableRow(myAlarm));
    }
  }
  response->print("</table><br/>\n");
  response->print("<a href=\"/new\">Create New</a>\n");
  response->print("</body></html>");
  request->send(response);
};


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
    oled.setString(4, "Hol");
  } else {
    oled.setString(4, "");
  }

  // Reload alarms from memory
  alarmsNeedUpdate = myAlarms.init();

  // Init button interrupts
  pinMode(SNOOZE_BUTTON, INPUT);
  attachInterrupt(SNOOZE_BUTTON, toggleSnooze, RISING);
  pinMode(OFF_BUTTON, INPUT);
  attachInterrupt(OFF_BUTTON, toggleOff, RISING);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });
  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleEdit(request);
  });
  server.on("/new", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleNew(request);
  });
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleSave(request);
  });
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleDelete(request);
  });

  server.onNotFound(notFound);
  server.begin();

  Serial.println("Up and runnning");
}


void loop() {
  delay(100);  // 0.1s

  // Update main screen
  oled.setString(0, myClock.getTimeString());
  oled.setString(1, myClock.getDateString());
  oled.setString(2, webContent.getWeatherString());

  if (myAlarms.isSnoozed()) {
    oled.setString(3, "Zzz");
  } else {
    oled.setString(3, "");
  }

  if (webContent.isTodayAHoliday()) {
    oled.setString(4, "Hol");
  } else {
    oled.setString(4, "");
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
    if (myAlarms.isOn()) {
      Serial.println("Alarming snooze button noticed");
      myAlarms.snooze();
    } else if (myAlarms.isSnoozed()){
      Serial.println("Already snoozing");
    } else {
      Serial.println("Not alarming snooze button noticed");
      oled.setString(1, "http://" + network.ipAddress());
      delay(5000);
    }
    snoozeHit = false;
  }
  if (offHit) {
    if (myAlarms.isOn() || myAlarms.isSnoozed()) {
      Serial.println("Alarming/Snoozing button off noticed");
      myAlarms.turnOff();
    } else {
      Serial.println("Not alarming button off noticed");
      oled.setString(1, "Fuck off!");
      delay(1000);
    }
    offHit = false;
  }
  if (alarmsNeedUpdate) {
    alarmsNeedUpdate = myAlarms.init();
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
