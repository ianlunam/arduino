#include "XPT2046_Touchscreen.h"
#include <SPI.h>

#define CS_PIN  33
#define MOSI 23
#define MISO 19
#define SCK 18

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  35
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void setup() {
  Serial.begin(115200);
  ts.begin();
  // ts.begin(SPI1); // use alternate SPI port
  ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
}

void loop() {

  // tirqTouched() is much faster than touched().  For projects where other SPI chips
  // or other time sensitive tasks are added to loop(), using tirqTouched() can greatly
  // reduce the delay added to loop() when the screen has not been touched.
  if (ts.tirqTouched()) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      Serial.print("Pressure = ");
      Serial.print(p.z);
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);
      delay(30);
      Serial.println();
    }
  }
}

