GPS Clock
===

Arduino sketch to pull the time from GPS satelllites. The GPS unit can take
about 5 minutes to aquire satellites, so it's backed by a real-time clock. 

Currently, the time zone offset is hard-coded (-6).  It is also DST by default;
ground D9 to return to standard time.

RTC, GPS, and OLED run at 3.3v. A level-shifter is used to raise the signals
to 5v for the Arduino.

Hardware Requirements
---

 - [Arduino Nano board](https://www.arduino.cc/en/Main/ArduinoBoardNano)
 - [SSD1306 I2C OLED Display - 128x32](https://www.adafruit.com/product/931)
 - [NEO-6M GPS Module](https://www.u-blox.com/en/product/neo-6-series)
 - [4-Channel Level Shifter](https://www.adafruit.com/product/757)
 - [DS3231 I2C Real-Time Clock](https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout/overview)
 - ~22 µF electrolytic capacitor, ~10 pF ceramic capacitor (line filter)
 - SPST latch button (for DST switch)

![Electrical Diagram](https://raw.githubusercontent.com/Pete-serversolved/arduino-gps-clock/master/diagrams/GPS-RTC-Clock-nano.png "GPS Clock diagram")
![Clock](https://raw.githubusercontent.com/Pete-serversolved/arduino-gps-clock/master/images/20180325_133028.jpg "Clock mounted on circuit board")
[![License: CC BY-SA 4.0](https://licensebuttons.net/l/by-sa/4.0/80x15.png)](https://creativecommons.org/licenses/by-sa/4.0/)
