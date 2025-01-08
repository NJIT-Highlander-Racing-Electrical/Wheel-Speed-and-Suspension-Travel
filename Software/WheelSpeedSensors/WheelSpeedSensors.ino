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

#define DEBUG_WHEEL false
#define DebugWheelSerial \
  if (DEBUG_WHEEL) Serial

#define DEBUG_SHOCK true
#define DebugShockSerial \
  if (DEBUG_SHOCK) Serial

#define frontLeftWheelPin 16
#define rearLeftWheelPin 17
#define frontRightWheelPin 18
#define rearRightWheelPin 19

#define frontLeftShockPin 13
#define rearLeftShockPin 33
#define frontRightShockPin 27
#define rearRightShockPin 32


// Wheel objects from Wheel.h definition
Wheel frontLeftWheel(frontLeftWheelPin);
Wheel rearLeftWheel(rearLeftWheelPin);
Wheel frontRightWheel(frontRightWheelPin);
Wheel rearRightWheel(rearRightWheelPin);

// Shock objects from Shock.h definition
Shock frontLeftShock(frontLeftShockPin);
Shock rearLeftShock(rearLeftShockPin);
Shock frontRightShock(frontRightShockPin);
Shock rearRightShock(rearRightShockPin);

void setup() {
  setupCAN(WHEEL_SPEED);
  Serial.begin(460800);

  // If the speed sensor detects a metal, it outputs a HIGH. Otherwise, LOW
  // Thus, we want to trigger interupt on LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(frontLeftWheel.sensorPin), frontLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearLeftWheel.sensorPin), rearLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(frontRightWheel.sensorPin), frontRightISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearRightWheel.sensorPin), rearRightISR, RISING);
}

void loop() {

  checkStatus(); // Checks status of all systems for error reporting to Base Station

  // updateWheelStatus calculates RPM if applicable, checks zero RPM status, and checks for wheelspin/wheel skid
  frontLeftWheel.updateWheelStatus();
  frontRightWheel.updateWheelStatus();
  rearLeftWheel.updateWheelStatus();
  rearRightWheel.updateWheelStatus();

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
  frontRightShock.getPosition();
  rearRightShock.getPosition();

  DebugShockSerial.print("frontLeftShock_Position:");
  DebugShockSerial.print(frontLeftShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("frontRightShock_Position:");
  DebugShockSerial.print(frontRightShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rearLeftShock_Position:");
  DebugShockSerial.print(rearLeftShock.reading);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rearRightShock_Position:");
  DebugShockSerial.print(rearRightShock.reading);
  DebugShockSerial.println();

}

void frontLeftISR() {
  // Set update flag so calculation is completed after we exit this ISR

  frontLeftWheel.updateFlag = true;
}

void rearLeftISR() {
  // Set update flag so calculation is completed after we exit this ISR
  rearLeftWheel.updateFlag = true;
}

void frontRightISR() {
  // Set update flag so calculation is completed after we exit this ISR
  frontRightWheel.updateFlag = true;
}

void rearRightISR() {
  // Set update flag so calculation is completed after we exit this ISR
  rearRightWheel.updateFlag = true;
}