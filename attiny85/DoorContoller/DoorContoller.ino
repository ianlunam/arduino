#include <ArduinoJson.h>

// // Include RadioHead Amplitude Shift Keying Library
// #include <RH_ASK.h>
// // Include dependant SPI Library
// #include <SPI.h>

// Button / Switch Pins
#define buttonPin 2  // PB2, pin 7, INT0

// Lamp and Light Dependant Resistor Pins
#define lampPin 0  // PB0, pin 5
#define ldrPin 3   // PB3, pin 2

// Motor Pins
#define motorControl 4  // PB4, pin 3

// Boundaries
#define low 100    // Low light boundary.
#define high 200   // High light boundary.
#define meh -1     // Default reading.
#define period 10  // Number of readings in array.

// Starting points
boolean open = false;
boolean stopEverything = false;
float readings[period];
int pointer = 0;

// // Create Amplitude Shift Keying Object
// RH_ASK rf_driver;

// ########################################
// setup() is run before first run of loop()
// ########################################
void setup() {
  GIMSK |= (1 << INT0);   // enable external interrupt
  MCUCR |= (1 << ISC00);  // CHANGE mode

  pinMode(lampPin, OUTPUT);
  pinMode(motorControl, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ldrPin, INPUT);

  // Init readings array to all meh
  for (int i = 0; i < period; i++) {
    readings[i] = meh;
  }

  // // Init the 433 driver
  // rf_driver.init();
}

// Loop control counter
int counter = 0;

// ########################################
// MAIN LOOP
// ########################################
void loop() {

  if ((counter % 300) == 0) {
    getReading();
  }

  if ((counter % 3000) == 0) {
    if (!stopEverything) {
      setStatus();
    }
    // sendStatus();
  }
  
  // ################################
  // Loop control
  // ################################
  // Loop period
  delay(100);
  // Loop counter
  //  Note: resetting counter to 1 if over 3000 prevents number from forever climbing.
  //        3000 is ((multiple of all functionTimers.frequencies) * (1000/loop period))
  counter++;
  if (counter > 3000) counter = 1;
  // ################################
}
// ########################################

ISR(INT0_vect) {
  int i = digitalRead(buttonPin);
  if (i == 1) {
    stopEverything = !stopEverything;
    openDoor(!open);
  }
}

void getReading() {
  // Get reading
  int c = analogRead(ldrPin);
  if (pointer >= period) pointer = 0;
  readings[pointer++] = c;
}

// Set the door status
//   get average reading for period
//   if average > high, if door closed, open it
//   if average < low, if door open, close it
void setStatus() {
  int average = getAverageLdrReading();
  if (average > high) {
    if (!open) {
      openDoor(true);
    }
  } else if (average < low) {
    if (open) {
      openDoor(false);
    }
  }
}

// Open (or close) the door
void openDoor(boolean newStatus) {
  if (newStatus) {
    open = true;
    digitalWrite(motorControl, HIGH);
    digitalWrite(lampPin, HIGH);
  } else {
    open = false;
    digitalWrite(motorControl, LOW);
    digitalWrite(lampPin, LOW);
  }
}

// // Build json object and send via 433 (or http)
// void sendStatus() {
//   StaticJsonDocument<200> doc;

//   // Who am i
//   doc["id"] = 254;
//   doc["model"] = "ChickenRun";
//   doc["channel"] = 4;

//   // Any other readings that might be available
//   doc["temp"] = 20;
//   doc["hum"] = 60;
//   doc["bat"] = 1;

//   // Get readings
//   doc["ldrAvg"] = getAverageLdrReading();
//   doc["ldrCur"] = getCurrentLdrReading();

//   // Get states
//   doc["open"] = open;
//   doc["stop"] = stopEverything;

//   String output = "";
//   serializeJson(doc, output);
//   // TODO: send somewhere using device
//   Serial.println(output);

//   // int str_len = output.length() + 1;
//   // char *char_array[str_len];
//   // output.toCharArray((uint8_t *)char_array, str_len);
//   // rf_driver.send(char_array);
// }

// Total all non-meh values and average
int getAverageLdrReading() {
  int total = 0;
  int counter = 0;
  for (int i = 0; i < period; i++) {
    if (readings[i] != meh) {
      total += readings[i];
      counter++;
    }
  }
  return total / counter;
}

// Get the most recent reading
int getCurrentLdrReading() {
  int pos = pointer - 1;
  if (pos < 0) pos = period - 1;
  return readings[pos];
}
