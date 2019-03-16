# NewPingESP8266
#### NewPing port for ESP8266 (Arduino IDE)

NewPing Library (Ultrasonic Sensors)

Written by Tim Eckel.

https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home

Modified for Teensy 3.0 & 3.1

https://github.com/PaulStoffregen/NewPing

http://forum.pjrc.com/threads/25907-Multiple-HCSR-04-Ultrasonic-sensors-on-teensy-3

![Photo](https://raw.githubusercontent.com/PaulStoffregen/NewPing/master/extras/NewPing_photo.jpg)

![Screenshot](https://raw.githubusercontent.com/PaulStoffregen/NewPing/master/extras/NewPing_screenshot.png)

Updated to support ESP8266 (Arduino IDE) by Jordan Shaw

https://github.com/jshaw/NewPingESP8266

Port was influenced by the issue: https://github.com/PaulStoffregen/NewPing/issues/2.

Also, removed non-compatable ping examples using timers due to incompatability with "Timer interrupt ping methods (won't work with non-AVR, ATmega128 and all ATtiny microcontrollers)" (NewPingESP8266.cpp ln. 198)
