#include "src/libraries/BajaCAN.h"  // https://arduino.github.io/arduino-cli/0.35/sketch-specification/#src-subfolder
#include "Wheel.h"
#include "Shock.h"

/*
*
*  NOTE: Maybe something should be added in this program that does the following:
*  
*  If the elapsed time is less than we would ever expect to see (at the fastest RPM),
*  then ignore the reading. Don't calculate RPM, but set the read time to 
*  lastReadingMillis so that we have a starting point for when we see another rev.
*
*/

#define DEBUG_WHEEL true
#define DebugWheelSerial \
  if (DEBUG_WHEEL) Serial

#define DEBUG_SHOCK true
#define DebugShockSerial \
  if (DEBUG_SHOCK) Serial


// Pin definitions for Wheels
const int frontLeftWheelPin = 16;
const int rearLeftWheelPin = 17;
const int frontRightWheelPin = 18;
const int rearRightWheelPin = 19;

// Pin definitions for Shocks
const int frontLeftShockPin = 13;
const int rearLeftShockPin = 33;
const int frontRightShockPin = 27;
const int rearRightShockPin = 32;

// Wheel objects from Wheel.h definition
Wheel frontLeftWheel(frontLeftWheelPin);
Wheel frontRightWheel(frontRightWheelPin);
Wheel rearLeftWheel(rearLeftWheelPin);
Wheel rearRightWheel(rearRightWheelPin);

// Shock objects from Shock.h definition
Shock frontLeftShock(frontLeftShockPin, true, frontLeftShock_restReading);
Shock frontRightShock(frontRightShockPin, true, frontRightShock_restReading);
Shock rearLeftShock(rearLeftShockPin, false, rearLeftShock_restReading);
Shock rearRightShock(rearRightShockPin, false, rearRightShock_restReading);

void setup() {
  setupCAN(WHEEL_SPEED);
  Serial.begin(460800);

  // If the speed sensor detects a metal, it outputs a HIGH. Otherwise, LOW
  // Thus, we want to trigger interupt on LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(frontLeftWheel.sensorPin), frontLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(frontRightWheel.sensorPin), frontRightISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearLeftWheel.sensorPin), rearLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearRightWheel.sensorPin), rearRightISR, RISING);
}

void loop() {

  checkStatus();  // Checks status of all systems for error reporting to Base Station

  // updateWheelStatus calculates RPM if applicable, checks zero RPM status, and checks for wheelspin/wheel skid
  frontLeftWheel.updateWheelStatus();
  frontRightWheel.updateWheelStatus();
  rearLeftWheel.updateWheelStatus();
  rearRightWheel.updateWheelStatus();

  // update CAN-Bus variables

  frontLeftWheelRPM = frontLeftWheel.rpm;
  frontRightWheelRPM = frontRightWheel.rpm;
  rearLeftWheelRPM = rearLeftWheel.rpm;
  rearRightWheelRPM = rearRightWheel.rpm;

  frontLeftWheelState = frontLeftWheel.wheelState;
  frontRightWheelState = frontRightWheel.wheelState;
  rearLeftWheelState = rearLeftWheel.wheelState;
  rearRightWheelState = rearRightWheel.wheelState;

  frontLeftDisplacement = frontLeftShock.wheelPos;  // These four must become displacement eventually once we have LookupTable configured
  frontRightDisplacement = frontRightShock.wheelPos;
  rearLeftDisplacement = rearLeftShock.wheelPos;
  rearRightDisplacement = rearRightShock.wheelPos;

  // Print data to serial monitor

  DebugWheelSerial.print("frontLeftWheel_RPM:");
  DebugWheelSerial.print(frontLeftWheel.rpm);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("frontRightWheel_RPM:");
  DebugWheelSerial.print(frontRightWheel.rpm);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("rearLeftWheel_RPM:");
  DebugWheelSerial.print(rearLeftWheel.rpm);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("rearRightWheel_RPM:");
  DebugWheelSerial.print(rearRightWheel.rpm);
  DebugWheelSerial.println();

  frontLeftShock.getPosition();
  frontRightShock.getPosition();
  rearLeftShock.getPosition();
  rearRightShock.getPosition();

  DebugShockSerial.print("frontLeftShock_reading:");
  DebugShockSerial.print(frontLeftShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("frontRightShock_reading:");
  DebugShockSerial.print(frontRightShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rearLeftShock_reading:");
  DebugShockSerial.print(rearLeftShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rearRightShock_reading:");
  DebugShockSerial.print(rearRightShock.reading);
  DebugShockSerial.println();
}

void frontLeftISR() {
  // Set update flag so calculation is completed after we exit this ISR

  frontLeftWheel.updateFlag = true;
}

void frontRightISR() {
  // Set update flag so calculation is completed after we exit this ISR
  frontRightWheel.updateFlag = true;
}


void rearLeftISR() {
  // Set update flag so calculation is completed after we exit this ISR
  rearLeftWheel.updateFlag = true;
}

void rearRightISR() {
  // Set update flag so calculation is completed after we exit this ISR
  rearRightWheel.updateFlag = true;
}