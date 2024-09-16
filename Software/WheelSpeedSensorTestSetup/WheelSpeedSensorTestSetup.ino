#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"

#define TFT_DC 21
#define TFT_CS 5

Adafruit_GC9A01A tft(TFT_CS, TFT_DC);

const int speedSensorPin = 12;
int targetsPerRevolution = 1;

unsigned long lastReadingMillis = 1;
unsigned long currentReadingMillis = 1;

float rpm = 0;

bool updateFlag = false;



void setup() {
  Serial.begin(115200);

  pinMode(speedSensorPin, INPUT);
  //
  // If the speed sensor detects a metal, it outputs a HIGH
  // Otherwise, it outputs a LOW
  //
  // Thus, we want to trigger interupt on LOW
  //
  attachInterrupt(digitalPinToInterrupt(speedSensorPin), updateRPM, FALLING);

  tft.begin();

  tft.fillScreen(GC9A01A_BLACK);
}


void loop(void) {

  if (updateFlag) {

    // Show that a revolution was detected
    Serial.println("Revolution detected!");
    // Clear screen before updating
    tft.fillScreen(GC9A01A_BLACK);

    // Print data to LCD
    tft.setCursor(50, 100);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(5);
    tft.println((int)rpm);

    tft.setCursor(100, 175);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(3);
    tft.println("RPM");
  
    // Clear the update flag
    updateFlag = false;
  
  }
}

void updateRPM() {

  // Set lastReadingMillis to the most recent reading
  lastReadingMillis = currentReadingMillis;

  // Then, update currentReadingMillis with the reading that triggered this condition
  currentReadingMillis = millis();

  // Calculate the new RPM value
  rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;

  updateFlag = true;
}
