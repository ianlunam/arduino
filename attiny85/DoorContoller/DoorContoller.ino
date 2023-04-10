#include <TinyWireS.h>

// Pins
#define ldrPin 3        // PB3, pin 2
#define motorControl 4  // PB4, pin 3

// Boundaries
#define low 100    // Low light boundary. Needs tuning.
#define high 200   // High light boundary. Needs tuning.
#define meh -1     // Default reading.
#define period 10  // Number of readings in array.
#define SLAVE_ADDR 0x4

const int INTERRUPT_PIN = 1;  // D3: pin 2, PB3

// Starting points
boolean open = false;
boolean stopEverything = false;
float readings[period];
int pointer = 0;
volatile bool button_flag = false;
volatile int counter = 0;

void setup() {
  enablePinChangeInterrupt();
  pinMode(motorControl, OUTPUT);
  pinMode(ldrPin, INPUT);

  // Init readings array to all meh
  for (int i = 0; i < period; i++) {
    readings[i] = meh;
  }

  TinyWireS.begin(SLAVE_ADDR);
  TinyWireS.onRequest(requestEvent);

  getReading();
  setStatus();
}


// ##########################################################
// MAIN LOOP, does nothing. Everything happens via interrupts
// ##########################################################
void loop() {
  TinyWireS_stop_check();
  counter++;

  getReading();

  if ((counter % 10) == 0) {
    if (!stopEverything) {
      setStatus();
    }
    // sendStatus();
    counter = 0;
  }

  if (button_flag) {
    digitalWrite(motorControl, HIGH);
    delay(1000);
    digitalWrite(motorControl, LOW);
    stopEverything = !stopEverything;
    openDoor(!open);
    button_flag = false;
  }

  delay(3000);
}
// ##########################################################


// Button interrupt
ISR(PCINT0_vect) {
  button_flag = true;
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
  } else {
    open = false;
    digitalWrite(motorControl, LOW);
  }
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

void requestEvent() {
  TinyWireS.write(getAverageLdrReading() / 4);
  TinyWireS.write(open);
}

/*
 * Command: enable pin change interrupts
 */
void enablePinChangeInterrupt() {
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  cli();
  PCMSK |= (1 << digitalPinToPCMSKbit(INTERRUPT_PIN)); // Pin Change Enable
  // equivalent to: PCMSK |= (1 <<PCINT3);
  GIMSK |= (1 << digitalPinToPCICRbit(INTERRUPT_PIN)); // PCIE Pin Change Interrupt Enable
  // equivalent to: GIMSK |= (1 << PCIE);
  sei();
}
