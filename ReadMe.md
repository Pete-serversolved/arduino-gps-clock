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

![Electrical Diagram](https://byron-supertech.visualstudio.com/ef56f05e-3922-43c2-ba28-69bc7d33c53b/_api/_versioncontrol/itemContent?repositoryId=392eab30-28de-4788-9596-0595671727a2&path=%2Fdiagrams%2FGPS-RTC-Clock-nano.png&version=GBmaster&contentOnly=true&__v=5 "GPS Clock diagram")
![Clock](https://byron-supertech.visualstudio.com/ef56f05e-3922-43c2-ba28-69bc7d33c53b/_api/_versioncontrol/itemContent?repositoryId=392eab30-28de-4788-9596-0595671727a2&path=%2Fimages%2F20180325_133028.jpg&version=GBmaster&contentOnly=true&__v=5)
[![License: CC BY-SA 4.0](https://licensebuttons.net/l/by-sa/4.0/80x15.png)](https://creativecommons.org/licenses/by-sa/4.0/)
