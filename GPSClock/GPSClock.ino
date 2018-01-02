#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <TimeLib.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define DIM_THRESHOLD 900

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

SoftwareSerial ss(8, 11); // TX from module on D7 (RX unused)
TinyGPS gps;
time_t prevDisplay = 0;
boolean use12Hour = false;
unsigned long lastUpdate = 0;

void gpsdump(TinyGPS &gps);
void printFloat(double f, int digits = 2);

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  
  delay(1000);

  Serial.println("uBlox Neo 6M");
  Serial.print("Testing TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println("by Mikal Hart");
  Serial.println();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for GPS...");
  display.display();
  unsigned long start = millis();
  while (ss.available() && millis() - start < 15000)     
  {
    char c = ss.read();
    //Serial.print(c);
    if (gps.encode(c)) 
    {
      updateTime(gps);
      break;
    }
  }
}

void loop() {
  bool newdata = false;
  display.setTextSize(2);
  display.setCursor(0,0);
  display.clearDisplay();
  unsigned long start = millis();
  // Every 5 seconds we print an update
  if(millis() - lastUpdate > 5000) 
  {
    while (ss.available()) 
    {
      char c = ss.read();
      //Serial.print(c);  // uncomment to see raw GPS data
      if (gps.encode(c)) 
      {
        newdata = true;
        break;
      }
    }
  }
  
  if (newdata) 
  {
    updateTime(gps);
/*
    Serial.println("Acquired Data");
    Serial.println("-------------");
    gpsdump(gps);
    Serial.println("-------------");
    Serial.println();
*/    
  }

  if( now() != prevDisplay){
    prevDisplay = now();
    // TODO: send the time to the display
    if(lastUpdate > 0)
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
  Serial.print("  Time: "); Serial.print(static_cast<int>(hr-6));  Serial.print(":"); //Serial.print("UTC -06:00 Chicago");
    Serial.print(static_cast<int>(minu)); Serial.print(":"); Serial.print(static_cast<int>(sec));
    Serial.print("."); Serial.print(static_cast<int>(hundredths)); Serial.print(" UTC -06:00 Chicago");
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
  gps.crack_datetime(&yr, &mnth, &dy, &hr, &minu, &sec, &hundredths, &age);
  byte localHour = (hr >= 6) ? hr - 6 : hr + 18;
  if(!(minute() == minu && hour() == localHour)) {
    Serial.println();
    Serial.println("=== Updating time to match GPS ===");
    Serial.print("Current: "); 
    Serial.print(hour()); 
    Serial.print(":"); 
    if(minute() < 10)
      Serial.print("0");
    Serial.println(minute());
    setTime(localHour, minu, sec, dy, mnth, yr);
    lastUpdate = millis();
    Serial.print("Updated: "); 
    Serial.print(hour()); 
    Serial.print(":"); 
    if(minute() < 10)
      Serial.print("0");
    Serial.println(minute());
    Serial.println();
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
void clockDisplay(){
  display.clearDisplay();
  if(use12Hour) {
    if(hourFormat12() < 10) {
      display.print(" ");
    }
    display.print(hourFormat12());
  } else {
    display.print("  ");
    if(hour() < 10)
      display.print("0");
    display.print(hour());
  }
  printDigits(minute());
  printDigits(second());
  if(use12Hour) {
    display.print(" ");
    display.print(isAM() ? "a" : "p");
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

