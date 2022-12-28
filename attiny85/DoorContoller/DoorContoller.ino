// All these are for timer interrupt
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// For JSON
#include <ArduinoJson.h>

// Pins
#define buttonPin 2     // PB2, pin 7, INT0
#define lampPin 0       // PB0, pin 5
#define ldrPin 3        // PB3, pin 2
#define motorControl 4  // PB4, pin 3

#define timer30 5       // PB5, pin 1, PCINT5
#define timer300 1       // PB1, pin 6, PCINT1

// Boundaries
#define low 100    // Low light boundary. Needs tuning.
#define high 200   // High light boundary. Needs tuning.
#define meh -1     // Default reading.
#define period 10  // Number of readings in array.

// Starting points
boolean open = false;
boolean stopEverything = false;
float readings[period];
int pointer = 0;
volatile bool button_flag = false;


void setup() {
  // Configure button interrupt
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

  set_sleep_mode(SLEEP_MODE_IDLE);

  // Setup timer1 to interrupt every second
  //   8-bit timer at 1MHz max is just over a second (1s == 243, max == 255)
  //   Code from https://www.instructables.com/Arduino-Timer-Interrupts/
  TCCR1 = 0;            // Stop timer
  TCNT1 = 0;            // Zero timer
  GTCCR = _BV(PSR1);    // Reset prescaler
  OCR1A = 243;          // T = prescaler / 1MHz = 0.004096s; OCR1A = (1s/T) - 1 = 243
  OCR1C = 243;          // Set to same value to reset timer1 to 0 after a compare match
  TIMSK = _BV(OCIE1A);  // Interrupt on compare match with OCR1A

  // Start timer in CTC mode; prescaler = 4096;
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS10);
  sei();
}

// Button interrupt
ISR(INT0_vect) {
  button_flag = true;
}

// Loop control counter
int counter = 0;

// Timer interrupt
ISR(TIMER1_COMPA_vect) {
  // Loop counter
  //  Note: resetting counter to 1 if over 3000 prevents number from forever climbing.
  //        3000 is ((multiple of all functionTimers.frequencies) * (1000/loop period))
  counter++;
  if (counter > 3000) counter = 1;
}

// ##########################################################
// MAIN LOOP, does nothing. Everything happens via interrupts
// ##########################################################
void loop() {
  if ((counter % 30) == 0) {
    getReading();
  }

  if ((counter % 300) == 0) {
    if (!stopEverything) {
      setStatus();
    }
    // sendStatus();
  }

  if (button_flag) {
    stopEverything = !stopEverything;
    openDoor(!open);
    button_flag = false;
  }

  sleep_enable();
  sleep_cpu();
}
// ##########################################################


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
