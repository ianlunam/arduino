#include <ezButton.h>
#include <ArduinoJson.h>
#include <L298N.h>

// // Include RadioHead Amplitude Shift Keying Library
// #include <RH_ASK.h>
// // Include dependant SPI Library
// #include <SPI.h>


// Button / Switch Pins
#define buttonPin A2
#define lowerLimitPin A4
#define upperLimitPin A5

// Lamp and Light Dependant Resistor Pins
#define lampPin 7
#define ldrPin A0

// Motor Pins
#define motorUp 12
#define motorDown 11
#define motorControl 10

// Boundaries
#define low 100    // Low light boundary.
#define high 200   // High light boundary.
#define meh -1     // Default reading.
#define period 10  // Number of readings in array.

boolean open = false;
boolean stopEverything = false;
float readings[period];
int pointer = 0;

// Init all buttons / switches
ezButton button(buttonPin);
ezButton lowerSwitch(lowerLimitPin);
ezButton upperSwitch(upperLimitPin);

// Init motor with controller
L298N motor(motorControl, motorUp, motorDown);

// // Create Amplitude Shift Keying Object
// RH_ASK rf_driver;

void setup() {
  Serial.begin(9600);
  pinMode(lampPin, OUTPUT);
  pinMode(motorUp, OUTPUT);
  pinMode(motorDown, OUTPUT);
  pinMode(motorControl, OUTPUT);

  // Init readings array to all meh
  for (int i = 0; i < period; i++) {
    readings[i] = meh;
  }

  // Set motor to slow
  motor.setSpeed(20);

  // // Init the 433 driver
  // rf_driver.init();
}

int counter = 0;

// MAIN LOOP
void loop() {
  button.loop();

  // ####### TIME BASED EVENTS ######
  // Loop happens ~ 100ms, so times
  // are modulos of seconds * 100

  // 30 seconds approx.
  if (counter % 300 == 0) {
    getReading();
  }

  // 300 seconds approx.
  if (counter % 3000 == 0) {
    if (!stopEverything) {
      setStatus();
      checkDrift();
    }
    sendStatus();
  }

  // ################################

  delay(100);

  if (button.isPressed()) {
    openDoor(!open);
    stopEverything = !stopEverything;
    Serial.print("Stopped: ");
    if (stopEverything) {
      Serial.println("y");
    } else {
      Serial.println("n");
    }
  }

  counter++;
  if (counter > 30000) counter = 1;
}

void getReading() {
  // Get reading
  int c = analogRead(ldrPin);
  if (pointer >= period) pointer = 0;
  readings[pointer++] = c;

  // Report current reading
  Serial.print("ldrCur: ");
  Serial.print(c);
  Serial.print(" ldrAvg: ");
  Serial.println(getAverageLdrReading());
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

void openDoor(boolean newStatus) {
  if (newStatus) {
    Serial.println("Opening");
    moveDoor(upperSwitch, true);
    open = true;
    digitalWrite(lampPin, HIGH);
  } else {
    Serial.println("Closing");
    moveDoor(lowerSwitch, false);
    open = false;
    digitalWrite(lampPin, LOW);
  }
}

// Move the door in a direction until the limit is reached.
void moveDoor(ezButton &limit, boolean upwards) {

  // Start movement, slow
  if ( upwards ) {
    motor.forward();
  } else {
    motor.backward();
  }

  // Wait for it to get there
  limit.loop();
  while (limit.getState() != 0) {
    delay(100);
    limit.loop();
  }

  // Stop movement
  motor.stop();
}

// Build json object and send via 433 or http
void sendStatus() {
  StaticJsonDocument<200> doc;

  // Who am i
  doc["id"] = 254;
  doc["model"] = "ChickenRun";
  doc["channel"] = 4;

  // Get states
  upperSwitch.loop();
  lowerSwitch.loop();
  doc["upper"] = upperSwitch.getState();
  doc["lower"] = lowerSwitch.getState();

  // Any other readings that might be available
  doc["temp"] = 20;
  doc["hum"] = 60;
  doc["bat"] = 1;

  // Get readings
  doc["ldrAvg"] = getAverageLdrReading();
  doc["ldrCur"] = getCurrentLdrReading();

  // Get states
  doc["open"] = open;
  doc["stop"] = stopEverything;

  String output = "";
  serializeJson(doc, output);
  // TODO: send somewhere using device
  Serial.println(output);

  // int str_len = output.length() + 1;
  // char *char_array[str_len];
  // output.toCharArray((uint8_t *)char_array, str_len);
  // rf_driver.send(char_array);
}

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

// If the weight of the door has cause it to close a bit
//   or something has pushed it up a bit
// Might be helped by using a stepper motor or somehow applying drag to the spindle.
void checkDrift() {
  upperSwitch.loop();
  lowerSwitch.loop();

  if ( open && upperSwitch.getState() == 1 ) {
    Serial.println("State drift down.");
    // TODO: Something
    openDoor(true);
  }
  if ( !open && lowerSwitch.getState() == 1 ) {
    Serial.println("State drift up.");
    // TODO: Something
    openDoor(false);
  }
}
