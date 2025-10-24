#include "Wheel.h"
#include "Shock.h"
#include "BajaCAN.h"

#define DEBUG_WHEEL false
#define DebugWheelSerial \
  if (DEBUG_WHEEL) Serial

#define DEBUG_SHOCK true
#define DebugShockSerial \
  if (DEBUG_SHOCK) Serial

// Pin definitions for Wheels
const int frontLeftWheelPin = 19;
const int rearLeftWheelPin = 18;
const int frontRightWheelPin = 17;
const int rearRightWheelPin = 16;

// Pin definitions for Shocks
const int frontLeftShockPin = 32;
const int rearLeftShockPin = 27;
const int frontRightShockPin = 33;
const int rearRightShockPin = 13;

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
  Serial.begin(460800);

  delay(100);  // Brief delay for serial to stabilize

  // If the speed sensor detects a metal, it outputs a HIGH. Otherwise, LOW
  // Thus, we want to trigger interrupt on LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(frontLeftWheel.sensorPin), frontLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(frontRightWheel.sensorPin), frontRightISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearLeftWheel.sensorPin), rearLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearRightWheel.sensorPin), rearRightISR, RISING);

  setupCAN(WHEEL_SPEED, 10);  // sendInterval = 10 means that we will be sending 100 times per second

  Serial.println("Wheel Speed System Initialized");
}

void loop() {
  // updateWheelStatus calculates RPM if applicable, checks zero RPM status, and checks for wheelspin/wheel skid
  frontLeftWheel.updateWheelStatus();
  frontRightWheel.updateWheelStatus();
  rearLeftWheel.updateWheelStatus();
  rearRightWheel.updateWheelStatus();

  // Update CAN-Bus variables
  frontLeftWheelSpeed = frontLeftWheel.wheelSpeedMPH;
  frontRightWheelSpeed = frontRightWheel.wheelSpeedMPH;
  rearLeftWheelSpeed = rearLeftWheel.wheelSpeedMPH;
  rearRightWheelSpeed = rearRightWheel.wheelSpeedMPH;

  frontLeftWheelState = frontLeftWheel.wheelState;
  frontRightWheelState = frontRightWheel.wheelState;
  rearLeftWheelState = rearLeftWheel.wheelState;
  rearRightWheelState = rearRightWheel.wheelState;

  // Read shock positions
  frontLeftShock.getPosition();
  frontRightShock.getPosition();
  rearLeftShock.getPosition();
  rearRightShock.getPosition();

  frontLeftDisplacement = frontLeftShock.wheelPos;
  frontRightDisplacement = frontRightShock.wheelPos;
  rearLeftDisplacement = rearLeftShock.wheelPos;
  rearRightDisplacement = rearRightShock.wheelPos;

  // Print data to serial monitor
  DebugWheelSerial.print("frontLeftWheel_Speed:");
  DebugWheelSerial.print(frontLeftWheel.wheelSpeedMPH, 2);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("frontRightWheel_Speed:");
  DebugWheelSerial.print(frontRightWheel.wheelSpeedMPH, 2);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("rearLeftWheel_Speed:");
  DebugWheelSerial.print(rearLeftWheel.wheelSpeedMPH, 2);
  DebugWheelSerial.print(",");
  DebugWheelSerial.print("rearRightWheel_Speed:");
  DebugWheelSerial.print(rearRightWheel.wheelSpeedMPH, 2);
  DebugWheelSerial.println();

  DebugShockSerial.print("fl_pos:");
  DebugShockSerial.print(frontLeftShock.wheelPos);
  DebugShockSerial.print(",");
  DebugShockSerial.print("fr_pos:");
  DebugShockSerial.print(frontRightShock.wheelPos);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rl_pos:");
  DebugShockSerial.print(rearLeftShock.wheelPos);
  DebugShockSerial.print(",");
  DebugShockSerial.print("rr_pos:");
  DebugShockSerial.print(rearRightShock.wheelPos);
  DebugShockSerial.println();
}

// ISR implementations - now use the debounced handleInterrupt() method
void frontLeftISR() {
  frontLeftWheel.handleInterrupt();
}

void frontRightISR() {
  frontRightWheel.handleInterrupt();
}

void rearLeftISR() {
  rearLeftWheel.handleInterrupt();
}

void rearRightISR() {
  rearRightWheel.handleInterrupt();
}