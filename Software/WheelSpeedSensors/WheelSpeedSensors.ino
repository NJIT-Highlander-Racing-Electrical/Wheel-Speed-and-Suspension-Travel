#include "src/libraries/BajaCAN.h"  // https://arduino.github.io/arduino-cli/0.35/sketch-specification/#src-subfolder

/*
*
*  NOTE: Maybe something should be added in this program that does the following:
*  
*  If the elapsed time is less than we would ever expect to see (at the fastest RPM),
*  then ignore the reading. Don't calculate RPM, but set the read time to 
*  lastReadingMillis so that we have a starting point for when we see another rev.
*
*  Also Note: This code is old and based off a previous version of 2023-2024 Archive.
*  Changes have been made in that version and the ISR was made as short as possible, so
*  make sure this reflects that. We also have to make sure that all of our processing is
*  done before the next interrupt service routine is triggered to prevent any data errors
*
*/


/*

  We also need to implement suspension displacement code

*/

#define frontLeftWheelPin 16;
#define rearLeftWheelPin 17;
#define frontRightWheelPin 18;
#define rearRightWheelPin 19;

const float wheelDiameter = 23;      // Diameter of our wheels in inches
const int targetsPerRevolution = 4;  // number of sensing points per revolution on the wheel
const float wheelSpinThreshold = 5;  // Speed difference (mph) above GPS vehicle velocity where we will declare wheelspin
const float wheelSkidThreshold = 5;  // Speed difference (mph) below GPS vehicle velocity where we will declare skidding

const float rpmToMphFactor = wheelDiameter / 63360 * 3.1415 * 60;  // When wheel RPM is multiplied by this, it results in that wheel's linear speed in MPH

float vehicleSpeedMPH = 0;  // Since GPS velocity is given in m/s, this converts and stores to MPH

// At one mile per hour, we are moving 1.46 feet per second
// Our wheel circumference is 6 feet
// This means we have 0.243 wheel revolutions per second
// If we have 4 targets on the wheel, we have a target going by every 1.03 seconds
// So, If we do not see a reading in 1.25 seconds for example, we know our wheel speed should be zero
// This comes by doing 1/(0.243 * targetsPerRevolution), plus a small error threshold, say 25%
const int zeroTimeoutMS = (1.00 / (0.243 * (float)targetsPerRevolution)) * 1.25;


// Class that defines shared variables and functions between the four wheels
class Wheel {

public:

  int sensorPin;  // GPIO that sensor is hooked up to

  unsigned long lastReadingMillis;
  unsigned long currentReadingMillis;
  
  int rpm;  // variable to store calculated RPM value

  float wheelSpeedMPH;  // calculated wheel velocity for comparison with GPS vehicle velocity

  volatile bool updateFlag;  // updateFlag is used to know when to calculate a new RPM

  enum int wheelState {
    GOOD,
    SPIN,
    SKID
  };

  Wheel(int pinNumber) {
    sensorPin = pinNumber;
    lastReadingMillis = 0;
    currentReadingMillis = 0;
    rpm = 0;
    wheelSpeedMPH = 0;
    updateFlag = false;
    wheelState = GOOD;

    pinMode(sensorPin, INPUT);
  }


  // Calculates RPM based on elapsed time between last reading and current time
  // Only runs after respective ISR is triggered
  void calculateRPM() {
    if (updateFlag) {

      // Clear the update flag
      updateFlag = false;

      // Set lastReadingMillis to the most recent reading
      lastReadingMillis = currentReadingMillis;
      // Then, update currentReadingMillis with the reading that triggered this condition
      currentReadingMillis = millis();

      // Calculate the new RPM value
      if (currentReadingMillis != lastReadingMillis) {
        rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;
        wheelSpeedMPH = rpm * rpmToMphFactor;
      } else {
        Serial.print("Avoided Divide-By-Zero error, not updating rpm value");
        return;
      }


      Serial.print("Revolution detected on pin ");
      Serial.print(sensorPin);
      Serial.print("! RPM: ");
      Serial.println(rpm);
    }
  }

  // Checks to see if a certain period of time has passed since last reading
  // If we surpass that threshold, set the RPM to zero
  void checkZeroRPM() {
    if ((millis() - currentReadingMillis) > zeroTimeoutMS) {

      // Set last reading to millis() so we can bounce back once another reading is detected
      currentReadingMillis = millis();

      // Set rpm to zero
      rpm = 0;
      updateFlag = true;
    }
  }

  // Compares wheel speed to GPS vehicle speed to see if we have wheelspin or skidding
  void checkWheelState() {
    if ((wheelSpeedMPH - vehicleSpeedMPH) > wheelSpinThreshold) {
      wheelState = SPIN;
    } else if ((wheelSpeedMPH - vehicleSpeedMPH) > -wheelSkidThreshold) {
      wheelState = SKID;
    } else {
      wheelState = GOOD;
    }
  }

  // Runs all of the above functions for simpler calls in loop()
  void updateWheelStatus() {
    calculateRPM();
    checkZeroRPM();
    checkWheelState();
  }
};

Wheel frontLeftWheel(frontLeftWheelPin);
Wheel rearLeftWheel(rearLeftWheelPin);
Wheel frontRightWheel(frontRightWheelPin);
Wheel rearRightWheel(rearRightWheelPin);

void setup() {
  setupCAN(WHEEL_SPEED);
  Serial.begin(115200);

  // If the speed sensor detects a metal, it outputs a HIGH. Otherwise, LOW
  // Thus, we want to trigger interupt on LOW to HIGH transition
  attachInterrupt(digitalPinToInterrupt(frontLeftWheel.sensorPin), frontLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearLeftWheel.sensorPin), rearLeftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(frontRightWheel.sensorPin), frontRightISR, RISING);
  attachInterrupt(digitalPinToInterrupt(rearRightWheel.sensorPin), rearRightISR, RISING);
}

void loop() {

  // updateWheelStatus calculates RPM if applicable, checks zero RPM status, and checks for wheelspin/wheel skid
  frontLeftWheel.updateWheelStatus();
  frontRightWheel.updateWheelStatus();
  rearLeftWheel.updateWheelStatus();
  rearRightWheel.updateWheelStatus();
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