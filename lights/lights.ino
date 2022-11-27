#include <ezButton.h>
#include <ArduinoJson.h>


#define pushme 5
#define lamp 7
#define ldr A0
#define low 100
#define high 200
#define meh -1
#define period 10

const unsigned long wait = 1000 * 5;

boolean open = false;
float readings[period];
int pointer = 0;

ezButton button(pushme);  // create ezButton object that attach to pin 7;


void setup() {
  Serial.begin(9600);
  pinMode(lamp, OUTPUT);
  for (int i = 0; i < period; i++) {
    readings[i] = meh;
  }
}

int counter = 0;

void loop() {
  button.loop();
  // 30 seconds approx.
  if (counter % 300 == 0 ) {
    getReading();
  }
  // 300 seconds approx.
  if (counter % 3000 == 0) {
    setStatus();
    sendStatus();
  }
  delay(100);

  if (button.isPressed()) {
    Serial.println("The button is pressed");
    openDoor(!open);
    // delay(300000);
  }

  // if (button.isReleased())
  //   Serial.println("The button is released");

  counter++;
  if ( counter > 20000 ) counter = 1;
}

void getReading() {
  // Get reading
  int c = analogRead(ldr);
  if (pointer >= period) pointer = 0;
  readings[pointer++] = c;

  // Report current reading
  Serial.print("LDR Reading: ");
  Serial.println(c);
}

void setStatus() {
  int average = 0;
  int counter = 0;
  for (int i = 0; i < period; i++) {
    if (readings[i] > -1) {
      average += readings[i];
      counter++;
    }
  }
  average = average / counter;
  Serial.print("Average: ");
  Serial.print(average);
  Serial.print("  Pointer: ");
  Serial.println(pointer);

  if (average > high) {
    if (!open) {
      Serial.println("Over high and closed");
      openDoor(true);
    }
  } else if (average < low) {
    if (open) {
      Serial.println("Under low and open");
      openDoor(false);
    }
  }
}

void openDoor(boolean newStatus) {
  Serial.print("Moving door to ");
  if (newStatus) {
    Serial.println("Open");
    digitalWrite(lamp, HIGH);
    open = true;
  } else {
    Serial.println("Closed");
    open = false;
    digitalWrite(lamp, LOW);
  }
}

void sendStatus() {
  Serial.println("Sending data");

  StaticJsonDocument<200> doc;
  doc["temp"] = 20;
  doc["hum"] = 60;
  doc["upper"] = false;
  doc["lower"] = true;
  doc["open"] = open;
  doc["reading"] = readings[pointer];

}