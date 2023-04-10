#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// Pins
#define LDR_PIN 32     // GPIO 32 - Has pulldown
#define BUTTON_PIN 33  // GPIO 33 - Has pulldown
#define MOTOR_PIN 12   // GPIO 12 - Random pick
#define LED_PIN 27     // GPIO 27 - Random pick

// Boundaries
#define LOW_LIGHT 50       // Low light boundary. Needs tuning.
#define HIGH_LIGHT 100      // High light boundary. Needs tuning.
#define MEH_VALUE -1        // Default reading.
#define PERIOD_VALUE 10     // Number of readings in array.
#define DEBOUNCE_TIME 200   // ns for debounce
#define ONE_SECOND 1000000  // us in 1s

unsigned long timer_30_value = ONE_SECOND * 30;       // 30s
unsigned long timer_300_value = ONE_SECOND * 60 * 5;  // 5mins

// Starting points
int pointer = 0;                     // Pointer to next value in readings
float readings[PERIOD_VALUE];        // Array or readings
boolean doorOpen = false;            // Current state of door
boolean stopEverything = false;      // Button pressed to override system
volatile bool button_flag = false;   // Button has been pressed
unsigned long lastDebounceTime = 0;  // Last time the button was pressed

volatile bool timer30_flag = false;   // Timer has fired
volatile bool timer300_flag = false;  // Timer has fired

// Hardware timers
hw_timer_t *timer30 = NULL;
hw_timer_t *timer300 = NULL;

void IRAM_ATTR onTimer30() {
  timer30_flag = true;
}

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
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, buttonPressed, RISING);

  // Initialise readings array
  for (int i = 0; i < PERIOD_VALUE; i++) {
    readings[i] = MEH_VALUE;
  }
  getReading();
  setStatus();

  // WiFi Setup
  Preferences wifiCreds;
  wifiCreds.begin("wifiCreds", true);
  String ssid = wifiCreds.getString("ssid");
  String pwd = wifiCreds.getString("password");
  wifiCreds.end();

  WiFi.begin(ssid.c_str(), pwd.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("WiFi Signal: %d\n", WiFi.RSSI());

  sendStatus();
  
  // Setup timers
  timer30 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer30, &onTimer30, true);
  timerAlarmWrite(timer30, timer_30_value, true);
  timerAlarmEnable(timer30);

  timer300 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer300, &onTimer300, true);
  timerAlarmWrite(timer300, timer_300_value, true);
  timerAlarmEnable(timer300);
}

void loop() {
  if (button_flag) {
    button_flag = false;
    stopEverything = !stopEverything;
    doorOpen = !doorOpen;
    openDoor(doorOpen);
  }

  if (timer30_flag) {
    timer30_flag = false;
    getReading();
  }

  if (timer300_flag) {
    timer300_flag = false;
    if (!stopEverything) {
      setStatus();
    } else {
      Serial.println("Override in place. Not checking status.");
    }
    sendStatus();
  }

  delay(100);
}

void getReading() {
  // Get reading
  int c = analogRead(LDR_PIN);
  if (pointer >= PERIOD_VALUE) pointer = 0;
  readings[pointer++] = c;
  Serial.printf("Reading: %d\n", c);
}

int getAverage() {
  int total = 0;
  int readingCounter = 0;
  for (int i = 0; i < PERIOD_VALUE; i++) {
    if (readings[i] != MEH_VALUE) {
      total += readings[i];
      readingCounter++;
    }
  }

  int avg = 0;
  if (readingCounter != 0) {
    avg = total / readingCounter;
  }
  Serial.printf("Total: %d Count: %d Avg: %d\n", total, readingCounter, avg);
  return avg;
}

// Set the door status
//   get average reading for PERIOD_VALUE
//   if average > high, if door closed, open it
//   if average < low, if door open, close it
void setStatus() {
  int average = getAverage();
  if (average > HIGH_LIGHT) {
    if (!doorOpen) {
      openDoor(true);
    }
  } else if (average < LOW_LIGHT) {
    if (doorOpen) {
      openDoor(false);
    }
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
  doc["averageReading"] = getAverage();

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
