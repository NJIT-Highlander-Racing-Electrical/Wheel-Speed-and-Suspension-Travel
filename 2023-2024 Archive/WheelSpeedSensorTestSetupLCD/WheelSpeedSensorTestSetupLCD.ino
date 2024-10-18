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

int n = 10; // number of previous readings to use in averaging;
int lastRpmReadings[n] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int rpmArrayIndex = 0;

int timeoutThreshold = 2000;  // Time in ms without a reading before RPM display resets

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

  // If reached reading timeout
  if ((millis() - currentReadingMillis) > timeoutThreshold) {

    // Set last reading to millis()
    currentReadingMillis = millis();

    // Set rpm to zero
    rpm = 0;
    updateFlag = true;
  }
}

void updateRPM() {

  // Set lastReadingMillis to the most recent reading
  lastReadingMillis = currentReadingMillis;

  // Then, update currentReadingMillis with the reading that triggered this condition
  currentReadingMillis = millis();

  // Calculate the new RPM value
  rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;

/*

  // FRAMEWORK FOR AVERAGING LAST 10 VALUES:

  // update last readings array
  lastRpmReadings[rpmArrayIndex] = rpm;
  if (rpmArrayIndex == (n-1)) {
    rpmArrayIndex = 0;
  } else {
    rpmArrayIndex++;
  }

  // Calculate average of new set of n values in array

  // Also need to change the variable printed to display in display update function

*/

  updateFlag = true;
}
