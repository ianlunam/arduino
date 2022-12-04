# Me playing around.

Mostly the idea is to control a hen house door based on a LDR. I started off with an Arduino Uno and bits. Now I've replaced the Arduino with at attiny85.

Logic:
* DoorController:
  - If light stays above a given level for more than 5 mins, tell door controller to open  
  - If light stays below a given level for more than 5 mins, tell door controller to close
  - If button pressed, disable interest in light and move door to opposite of that it is
  - While door is open, have LED on

* Actual motor controller logic circuit.
  - Two reed switches, one at top and one at bottom of door. Triggered by magnet in door moving to top/bottom.
  - DC motor with enough power, connected to L298N controller
  - Only one input - high for door to be open, low for door to be closed
  - AND and Inverter logic ICs convert high/low and triggered reedswitchs into instructions for L298N

Arduino version in DoorController directory

attiny85 version in attiny85 directory
