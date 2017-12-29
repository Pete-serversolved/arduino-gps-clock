#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define DIM_THRESHOLD 900

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

SoftwareSerial mySerial(7, 11); // TX from module on D7 (RX unused)
TinyGPS gps;

void setup() {
  
}

void loop() {
  
}

