# Atmega alarm clock
Final project for microcontrollers course at AGH, Cracow. Clock uses Arduino Uno clone as a development board with Atmega microcontroler and seven segment display. The main purpos of this project was learning how to write code for AVRs using Atmel Studio and self-etching PCBs.

![](https://cloud.githubusercontent.com/assets/25593055/22661873/426e425e-eca7-11e6-8a26-224b269de68d.jpg)

## Description
The code allows for setting current time and date (including leap years) as well as setting alarm. It can also display temperature using Atmega328's internal sensor, though it cannot be described as accurate. A 4 axis joystick is used as user input.

![](https://cloud.githubusercontent.com/assets/25593055/22661872/426d56d2-eca7-11e6-8b02-292a9c5f22d8.png)

###BOM
- Arduiono Uno (or clone)
- common anode 7 segment display
- 4 axis joystic with button
- buzzer

Compiled hex file can be uploaded via Arduino Builder or ISP programmer (for example USBasp with 6 pin adapter).
