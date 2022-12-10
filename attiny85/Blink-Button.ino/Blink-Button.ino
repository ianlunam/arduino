#define LED_PIN 0
#define LDR_PIN 3
#define BUTTON_PIN 2

void setup() {
  GIMSK |= (1 << INT0);   // enable external interrupt
  MCUCR |= (1 << ISC00);  // CHANGE mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

boolean enableFlasher = true;

// the loop function runs over and over again forever
void loop() {
  int val = analogRead(LDR_PIN);
  if ( val > 200 && enableFlasher ) {
    digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                  // wait for a second
    digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
    delay(1000);
  }  // wait for a second
}

ISR(INT0_vect) {
  int i = digitalRead(BUTTON_PIN);
  if (i == 1) {
    enableFlasher = !enableFlasher;
  }
}