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
  //setupCAN(WHEEL_SPEED);
  Serial.begin(500000);

  // If the speed sensor detects a metal, it outputs a HIGH. Otherwise, LOW
  // Thus, we want to trigger interupt on LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(frontLeftWheel.sensorPin), frontLeftISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(rearLeftWheel.sensorPin), rearLeftISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(frontRightWheel.sensorPin), frontRightISR, RISING);
  //attachInterrupt(digitalPinToInterrupt(rearRightWheel.sensorPin), rearRightISR, RISING);
}

void loop() {

  // updateWheelStatus calculates RPM if applicable, checks zero RPM status, and checks for wheelspin/wheel skid
  frontLeftWheel.updateWheelStatus();
  //frontRightWheel.updateWheelStatus();
  //rearLeftWheel.updateWheelStatus();
  //rearRightWheel.updateWheelStatus();

  frontLeftShock.getPosition();
  //frontRightShock.getPosition();
  //rearLeftShock.getPosition();
  // rearRightShock.getPosition();
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