# attiny85 code

Programming done by folling instructions (here)[https://circuitdigest.com/microcontroller-projects/programming-attiny85-microcontroller-ic-using-arduino].

NOTE: In Arduino IDE v2 to upload to an IC via `Arduino as ISP` use `Sketch` -> `Upload Using Programmer` or ctl-shft-U.

## Wiring for programming attiny85 via Arduino Uno

![attiny85 pinout](../ATtiny85-Pinout.png)

ATtiny85 Pin -> Arduino Uno Pin

* Vcc -> 5V
* GND -> GND
* Pin 7 (PB2) -> 13
* Pin 6 (PB1) -> 12
* Pin 5 (PB0) -> 11
* Pin 1 (PB5, RESET) -> 10

## Programming attiny85 via Arduino Uno

* Connect Arduino Uno to PC
* Open Arduino IDE
* Select Arduino Uno and port
* Load `Examples/11.ArduinoISP/AdruinoISP` and upload to the Uno
* Change settings to be `attiny` and the same port
* Select desired Clock speed, Processor and Programmer (`Arduino as ISP`) in `Tools`
* `Burn Bootloader` under `Tools`

If you change the clock speedyou have to reburn the bootloader.

After that, it's all yours. Program away. To upload your code to the chip use
`Sketch` -> `Upload Using Programmer` otherwise it'll try to load it on the
Arduino and fail.
