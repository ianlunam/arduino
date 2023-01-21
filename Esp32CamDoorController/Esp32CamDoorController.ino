#include <WiFi.h>
#include <Preferences.h>

// Pins
#define ldrPin 15        // GPIO 16 - Only unassigned pin available
#define buttonPin 0      // GPIO 0 - Has to be low to boot, high to program
#define motorControl 12  // GPIO 12 - Random pick
#define flashLight 4     // GPIO 4 - On-board bright LED

// Boundaries
#define low 100    // Low light boundary. Needs tuning.
#define high 200   // High light boundary. Needs tuning.
#define meh -1     // Default reading.
#define period 10  // Number of readings in array.

// Starting points
volatile boolean doorOpen = true;
boolean stopEverything = false;
float readings[period];
int pointer = 0;
volatile bool button_flag = false;
volatile bool timer30_flag = false;
volatile bool timer300_flag = false;

hw_timer_t *timer30 = NULL;
hw_timer_t *timer300 = NULL;

void IRAM_ATTR onTimer30() {
  timer30_flag = true;
}

void IRAM_ATTR onTimer300() {
  timer300_flag = true;
}

void IRAM_ATTR buttonPressed() {
  button_flag = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  pinMode(motorControl, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ldrPin, INPUT);

  for (int i = 0; i < period; i++) {
    readings[i] = meh;
  }
  attachInterrupt(buttonPin, buttonPressed, RISING);

  timer30 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer30, &onTimer30, true);
  timerAlarmWrite(timer30, 3000000, true);
  timerAlarmEnable(timer30);

  timer300 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer300, &onTimer300, true);
  timerAlarmWrite(timer300, 30000000, true);
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
    Serial.println("30 timer");
    timer30_flag = false;
    getReading();
  }

  if (timer300_flag) {
    Serial.println("300 timer");
    timer300_flag = false;
    if (!stopEverything) {
      setStatus(getAverage());
      sendPhoto();
    }
  }

  delay(100);
}

void getReading() {
  // Get reading
  int c = analogRead(ldrPin);
  if (pointer >= period) pointer = 0;
  readings[pointer++] = c;
  Serial.printf("Reading: %d\n", c);
}

int getAverage() {
  int total = 0;
  int readingCounter = 0;
  for (int i = 0; i < period; i++) {
    if (readings[i] != meh) {
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
//   get average reading for period
//   if average > high, if door closed, open it
//   if average < low, if door open, close it
void setStatus(int average) {
  if (average > high) {
    if (!doorOpen) {
      openDoor(true);
    }
  } else if (average < low) {
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
    digitalWrite(motorControl, HIGH);
  } else {
    Serial.println("Closing");
    doorOpen = false;
    digitalWrite(motorControl, LOW);
  }
}

void sendPhoto() {
  // ###############
  // Wifi Connection
  // ###############
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

  Serial.println(WiFi.RSSI());
  WiFi.disconnect();


  pinMode(ldrPin, INPUT);
}
