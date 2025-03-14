/*

  We would like to report wheel displacement using these LPPS-22-200 Linear Travel Sensors.
  This involves finding the value of the sensor at rest, and determining how much the signal
  changes as it moves a certain distance. Then, by using the motion ratio of the suspension
  system, we can determine the wheel displacement from the suspension suspension displacement 

*/

// The motion ratio defines the ratio between shock travel and wheel travel
const float frontMotionRatio = 0.75;
const float rearMotionRatio = 0.82;

// Experimentally-found rest values of shocks
const int frontLeftShock_restReading = 2048;
const int frontRightShock_restReading = 2048;
const int rearLeftShock_restReading = 2048;
const int rearRightShock_restReading = 2048;

// The number of analogRead integer steps that is linearly related to inches traveled of sensor
const int analogValPerInch = -1;

// Class that defines shared variables and functions between the four wheels
class Shock {

private:

  bool frontShock;  // Set true if the given shock is on the front of the car, otherwise false
  int reading;      // Analog reading value from ESP32
  int sensorPin;    // GPIO that sensor is hooked up to
  int restReading;  // Position of the given shock while the vehicle is at rest/ride height
  float shockPos;   // Inches that shock has traveled from rest position


public:

  float wheelPos;   // Inches that wheel has traveled from rest position

  Shock(int pinNumber, bool isFrontShock, int restPositionVal) {
    sensorPin = pinNumber;
    frontShock = isFrontShock;
    restReading = restPositionVal;
    reading = 0;
    wheelPos = 0;
    shockPos = 0;
    pinMode(sensorPin, INPUT);
  }


  void getPosition() {

    // Get initial analog reading
    reading = analogRead(sensorPin);
    delay(2);
    reading = analogRead(sensorPin);

    if (analogValPerInch == -1) {
      Serial.println("You forgot to set the analogValPerInch to something useful dummy");
    }

    // Convert reading to inches that sensor has traveled from rest
    shockPos = (float)(reading - restReading) / (float)analogValPerInch;
    if (frontShock) {
      wheelPos = shockPos * frontMotionRatio;
    } else {
      wheelPos = shockPos * rearMotionRatio;
    }
  }
};
