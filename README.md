# Me playing around with hardware.

I've scored a job withing with robotics, so I figure I should learn *some* stuff
about logic circuits and microcontrollers before I start. I don't need to know a lot
as I'm a DevOps engineer, so I'll just be doing build pipelines and stuff, but hey! Why not?

Started as the control of a hen house door based on a LDR. Bought an Arduino Uno
and bits. Now I've replaced the Arduino with an attiny85, and started plying with
logic chips.

Original Arduino Uno version with ezButton, loops, structs is [here](./DoorContoller).

New version for attiny with interrupts, sleeping CPUs and logic ICs is [here](./attiny85/DoorContoller).

Due to various requirements and limitations of the equipment, I'm now moving to having the attiny85 watch
the LDR and run the door control code, logic ICs to move and door and an ESP32-CAM as a webcam and to
report back to the house. The attiny85 will provide the esp32 with data via I2C.

Why? Because the esp32-cam doesn't have any analog pins available because they're all used by the camera.

