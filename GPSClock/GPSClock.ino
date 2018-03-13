#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>
#include "RTClib.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define DIM_THRESHOLD 900

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
volatile boolean tzNegative = true;
volatile boolean showTZAdjust = false;
boolean updateTZFromClock = false;
boolean gpsAcquired = false;

void gpsdump(TinyGPS &gps);
void printFloat(double f, int digits = 2);
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
  display.setRotation(2);
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
    updateTZFromClock = false;
    clockDisplay();
  }
}

void loop() {
  bool newdata = false;
  unsigned long start = millis();
  // Every second we print an update
  while (millis() - start < 500) 
  {
    if (ss.available()) 
    {
      char c = ss.read();
      if(!gpsAcquired)
        Serial.print(c);  // uncomment to see raw GPS data
        
      if (gps.encode(c)) 
      {
        newdata = true;
        gpsAcquired = true;
        break;  // uncomment to print new data immediately!
      }
    }
  }
  
  if (newdata) 
  {
    Serial.println("Acquired Data");
    Serial.println("-------------");
//    gpsdump(gps);
    updateTime(gps);
    Serial.println("-------------");
    Serial.println();
  }

  DateTime now = rtc.now();
  if( now.unixtime() != prevDisplay){
    prevDisplay = now.unixtime();
    // TODO: send the time to the display
    clockDisplay();
  }
}

void gpsdump(TinyGPS &gps)
{
  long lat, lon;
  float flat, flon;
  unsigned long age, date, time, chars;
  int yr;
  byte mnth, dy, hr, minu, sec, hundredths;
  unsigned short sentences, failed;

  gps.get_position(&lat, &lon, &age);
  Serial.print("Lat/Long(10^-5 deg): "); Serial.print(lat); Serial.print(", "); Serial.print(lon); 
  Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");
  
  // On Arduino, GPS characters may be lost during lengthy Serial.print()
  // On Teensy, Serial prints to USB, which has large output buffering and
  //   runs very fast, so it's not necessary to worry about missing 4800
  //   baud GPS characters.

  gps.f_get_position(&flat, &flon, &age);
  Serial.print("Lat/Long(float): "); printFloat(flat, 5); Serial.print(", "); printFloat(flon, 5);
    Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");

  gps.get_datetime(&date, &time, &age);
  Serial.print("Date(ddmmyy): "); Serial.print(date); Serial.print(" Time(hhmmsscc): ");
    Serial.print(time);
  Serial.print(" Fix age: "); Serial.print(age); Serial.println("ms.");
  gps.crack_datetime(&yr, &mnth, &dy, &hr, &minu, &sec, &hundredths, &age);
  Serial.print("Date: "); Serial.print(static_cast<int>(mnth)); Serial.print("/"); 
    Serial.print(static_cast<int>(dy)); Serial.print("/"); Serial.print(yr);
  Serial.print("  Time: "); Serial.print(static_cast<int>(hr));  Serial.print(":"); //Serial.print("UTC -06:00 Chicago");
    Serial.print(static_cast<int>(minu)); Serial.print(":"); Serial.print(static_cast<int>(sec));
    Serial.print("."); Serial.print(static_cast<int>(hundredths)); Serial.print(" UTC");
  Serial.print("  Fix age: ");  Serial.print(age); Serial.println("ms.");

  Serial.print("Alt(cm): "); Serial.print(gps.altitude()); Serial.print(" Course(10^-2 deg): ");
    Serial.print(gps.course()); Serial.print(" Speed(10^-2 knots): "); Serial.println(gps.speed());
  Serial.print("Alt(float): "); printFloat(gps.f_altitude()); Serial.print(" Course(float): ");
    printFloat(gps.f_course()); Serial.println();
  Serial.print("Speed(knots): "); printFloat(gps.f_speed_knots()); Serial.print(" (mph): ");
    printFloat(gps.f_speed_mph());
  Serial.print(" (mps): "); printFloat(gps.f_speed_mps()); Serial.print(" (kmph): ");
    printFloat(gps.f_speed_kmph()); Serial.println();

  gps.stats(&chars, &sentences, &failed);
  Serial.print("Stats: characters: "); Serial.print(chars); Serial.print(" sentences: ");
    Serial.print(sentences); Serial.print(" failed checksum: "); Serial.println(failed);
}

void updateTime(TinyGPS &gps) {
  unsigned long age;
  int yr;
  byte mnth, dy, hr, minu, sec, hundredths;
  isDST = digitalRead(dstPin);
  gps.crack_datetime(&yr, &mnth, &dy, &hr, &minu, &sec, &hundredths, &age);
  DateTime now = rtc.now();
  if(updateTZFromClock) {
    int diff = (hr - now.hour()) % 24;
    tzOffset = (diff > 12 ) ? (diff - 24) : ( (diff < -12) ? (diff + 24) : diff );
    Serial.println("Time zone updated from real-time clock.");    
    updateTZFromClock = false;
  } else {
    byte localHour = (hr + 24 - (tzOffset - (isDST ? 1 : 0))) % 24;
    Serial.print("RTC Hour: ");
    Serial.print(now.hour());
    Serial.print("; GPS Hour: ");
    Serial.print(hr);
    Serial.print("; Offset: ");
    if(tzNegative)
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
}

void printFloat(double number, int digits)
{
  // Handle negative numbers
  if (number < 0.0) 
  {
     Serial.print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  Serial.print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    Serial.print("."); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0) 
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    Serial.print(toPrint);
    remainder -= toPrint;
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
    //display.print("  ");
    if(now.hour() < 10)
      display.print("0");
    display.print(now.hour());
  }
  printDigits(now.minute());
  //printDigits(now.second());

  if(use12Hour) {
    display.print(" ");
    display.print(am ? "a" : "p");
  }
//  display.dim(dimDisplay);
  display.display();
}

// Utility function for clock display: prints preceding colon and leading 0
void printDigits(int digits){
  display.print(":");
  if(digits < 10) {
    display.print('0');
  }
  display.print(digits);
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

