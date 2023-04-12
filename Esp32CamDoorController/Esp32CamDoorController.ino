#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
// #include <time.h>
#include <sunset.h>


// Pins
#define BUTTON_PIN 33  // GPIO 33 - Has pulldown
#define MOTOR_PIN 12   // GPIO 12 - Random pick
#define LED_PIN 33     // GPIO 27 - Random pick

// Boundaries
#define LOW_LIGHT 50       // Low light boundary. Needs tuning.
#define HIGH_LIGHT 100      // High light boundary. Needs tuning.
#define MEH_VALUE -1        // Default reading.
#define PERIOD_VALUE 10     // Number of readings in array.
#define DEBOUNCE_TIME 200   // ns for debounce
#define ONE_SECOND 1000000  // us in 1s

unsigned long timer_300_value = ONE_SECOND * 30;  // 60 * 5;  // 5mins

const char* tz = "NZST-12NZDT,M9.5.0,M4.1.0/3";
// const char* tz = "UTC";

#define LATITUDE -35.122673
#define LONGITUDE 173.2629643

// Starting points
boolean doorOpen = false;             // Current state of door
boolean stopEverything = false;       // Button pressed to override system
volatile bool button_flag = false;    // Button has been pressed
unsigned long lastDebounceTime = 0;   // Last time the button was pressed
volatile bool timer300_flag = false;  // Has timer fired?

// Hardware timers
hw_timer_t* timer300 = NULL;
struct tm timeinfo;
SunSet sun;

void IRAM_ATTR onTimer300() {
  timer300_flag = true;
}

void IRAM_ATTR buttonPressed() {
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    button_flag = true;
    lastDebounceTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  // Define pin modes
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, buttonPressed, RISING);

  setStatus();

  // WiFi Setup
  Preferences wifiCreds;
  wifiCreds.begin("wifiCreds", true);
  String ssid = wifiCreds.getString("ssid");
  String pwd = wifiCreds.getString("password");
  wifiCreds.end();
  Serial.printf("SSID: %s Pwd: %s\n", ssid, pwd);

  WiFi.begin(ssid.c_str(), pwd.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("WiFi Signal: %d\n", WiFi.RSSI());

  configTzTime(tz, "nz.pool.ntp.org");
  Serial.println("NTP updated");
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  sun.setPosition(LATITUDE, LONGITUDE, (timeinfo.tm_isdst ? 13 : 12));

  // sendStatus();

  timer300 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer300, &onTimer300, true);
  timerAlarmWrite(timer300, timer_300_value, true);
  timerAlarmEnable(timer300);
  setStatus();
}

void loop() {
  if (button_flag) {
    button_flag = false;
    stopEverything = !stopEverything;
    doorOpen = !doorOpen;
    openDoor(doorOpen);
  }

  if (timer300_flag) {
    timer300_flag = false;
    if (!stopEverything) {
      setStatus();
    } else {
      Serial.println("Override in place. Not checking status.");
    }
    // sendStatus();
  }

  delay(100);
}

// Set the door status
//   get average reading for PERIOD_VALUE
//   if average > high, if door closed, open it
//   if average < low, if door open, close it
void setStatus() {
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  time_t now;
  time(&now);
  sun.setCurrentDate(timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
  sun.setTZOffset((timeinfo.tm_isdst ? 13 : 12));
  // sun.setCurrentDate(2023, 3, 31);

  double sunrise = sun.calcSunrise();
  double sunset = sun.calcSunset();
  Serial.printf("Year: %d Month: %d Day: %d Time: %d:%d:%d ST:%d\n", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_isdst);
  char ptr[60];
  strftime(ptr, 60, "%x %X %Y %z %Z", &timeinfo);
  Serial.print(ptr);
  Serial.printf("\nNow: %d\n", now);
  Serial.printf("Rise: %d ", sunset);
  Serial.printf("Set: %d ", sunrise);
  Serial.printf("Now: %d\n", now);

  strftime(ptr, 60, "%x %X %Y %z %Z", &timeinfo);
  Serial.printf("Ptr: %s\n", ptr);
  
  if (sunset < now && sunrise > now) {
    openDoor(true);
  } else {
    openDoor(false);
  }
}

// Open (or close) the door
void openDoor(boolean newStatus) {
  if (newStatus) {
    Serial.println("Opening");
    doorOpen = true;
    digitalWrite(MOTOR_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("Closing");
    doorOpen = false;
    digitalWrite(MOTOR_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

String boolToString(bool value) {
  if (value) return "True";
  return "False";
}

void sendStatus() {
  StaticJsonDocument<200> doc;
  doc["id"] = "ChickenRunDoorController";
  doc["doorOpen"] = boolToString(doorOpen);
  doc["override"] = boolToString(stopEverything);

  String json;
  serializeJson(doc, json);
  HTTPClient http;
  http.begin("http://192.168.0.241/upload.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Accept", "*/*");
  int httpCode = http.POST(json);
  Serial.print(http.getString());
  http.end();
}
