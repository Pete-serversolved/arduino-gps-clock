#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>
#include "RTClib.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

RTC_DS3231 rtc;
SoftwareSerial ss(8, 11); // TX from module on D8 (RX unused)
TinyGPS gps;

int prevDisplay = 0;
byte dstPin = 9;
boolean use12Hour = false;
boolean isDST = false;
unsigned long lastUpdate = 0;
volatile byte tzOffset = 6;
boolean gpsAcquired = false;

byte to12hour(byte hour, bool &am);
void printDigits(int digits);

void setup() {
#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  pinMode(dstPin, INPUT_PULLUP);
  isDST = digitalRead(dstPin);
  Serial.begin(115200);
  ss.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  Serial.println("uBlox Neo 6M");
  Serial.print("Testing TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println("by Mikal Hart");
  Serial.println();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if (rtc.lostPower()) {
    display.println("Waiting for GPS...");
    display.display();
    updateTime(gps);
  } else {
    prevDisplay = rtc.now().unixtime();
    clockDisplay();
  }
}

void loop() {
  bool newdata = false;
  unsigned long loopStart = millis();
  // Every second we print an update
  while (millis() - loopStart < 500) {
    if (ss.available()) {
      char c = ss.read();
      if(!gpsAcquired)
        Serial.print(c);  // See raw GPS data until it's recognized
        
      if (gps.encode(c)) {
        newdata = true;
        gpsAcquired = true;
        break;  // uncomment to print new data immediately!
      }
    }
  }
  
  if (newdata) {
    Serial.println("Acquired Data");
    Serial.println("-------------");
    updateTime(gps);
    Serial.println("-------------");
    Serial.println();
  }

  DateTime now = rtc.now();
  if( now.unixtime() != prevDisplay){
    prevDisplay = now.unixtime();
    clockDisplay();
  }
}

void updateTime(TinyGPS &gps) {
  unsigned long age;
  int yr;
  byte mnth, dy, hr, minu, sec, hundredths;
  isDST = digitalRead(dstPin);
  gps.crack_datetime(&yr, &mnth, &dy, &hr, &minu, &sec, &hundredths, &age);
  DateTime now = rtc.now();
  byte localHour = (hr + 24 - (tzOffset - (isDST ? 1 : 0))) % 24;
  Serial.print("RTC Hour: ");
  Serial.print(now.hour());
  Serial.print("; GPS Hour: ");
  Serial.print(hr);
  Serial.print("; Offset: ");
  Serial.print("-");
  Serial.print(abs(tzOffset - (isDST ? 1 : 0)));
  Serial.print("; Local hour is now ");
  Serial.println(localHour);
  if(!(now.second() == sec && now.minute() == minu && now.hour() == localHour)) {
    Serial.println("Updating real-time clock ...");
    rtc.adjust(DateTime(yr, mnth, dy, localHour, minu, sec));
  } else {
    Serial.println("GPS Time matches real-time clock.");
  }
}

// Clock display of the time and date (Basic)
// reads directly from RTC and displays
void clockDisplay(){
  bool am;

  display.setTextSize(4);
  display.setCursor(0,0);
  display.clearDisplay();
  DateTime now = rtc.now();

  if(use12Hour) {
    byte hour12 = to12hour(now.hour(), am);
    if(hour12 < 10) {
      display.print(" ");
    }
    display.print(hour12);
  } else {
    if(now.hour() < 10) {
      display.print("0");
    }
    display.print(now.hour());
  }
  display.print(":");
  if(now.minute() < 10) {
    display.print('0');
  }
  display.print(now.minute());

  if(use12Hour) {
    display.print(" ");
    display.print(am ? "a" : "p");
  }
  display.display();
}

byte to12hour(byte hour, bool &am) {
  if(hour == 0) {
    am = true;
    return 12;
  }
  if(hour <= 12) {
    am = true;
    return hour;
  } else {
    am = false;
    return hour - 12;
  }
}

