const float wheelDiameter = 23;      // Diameter of our wheels in inches
const int targetsPerRevolution = 4;  // number of sensing points per revolution on the wheel
const float wheelSpinThreshold = 5;  // Speed difference (mph) above GPS vehicle velocity where we will declare wheelspin
const float wheelSkidThreshold = 5;  // Speed difference (mph) below GPS vehicle velocity where we will declare skidding

const float rpmToMphFactor = wheelDiameter / 63360.0 * 3.1415 * 60.0;  // When wheel RPM is multiplied by this, it results in that wheel's linear speed in MPH

float vehicleSpeedMPH = 0;  // Since GPS velocity is given in m/s, this converts and stores to MPH

// At one mile per hour, we are moving 1.46 feet per second
// Our wheel circumference is 6 feet
// This means we have 0.243 wheel revolutions per second
// If we have 4 targets on the wheel, we have a target going by every 1.03 seconds
// So, If we do not see a reading in 1.25 seconds for example, we know our wheel speed should be zero
// This comes by doing 1/(0.243 * targetsPerRevolution), plus a small error threshold, say 25%, and then converting from seconds to ms
const int zeroTimeoutMS = (1.00 / (0.243 * (float)targetsPerRevolution)) * 1.25 * 1000;

enum WheelState {
  GOOD,
  SPIN,
  SKID
};



// Class that defines shared variables and functions between the four wheels
class Wheel {

public:

  int sensorPin;  // GPIO that sensor is hooked up to

  unsigned long lastReadingMillis;
  unsigned long currentReadingMillis;

  int rpm;  // variable to store calculated RPM value

  float wheelSpeedMPH;  // calculated wheel velocity for comparison with GPS vehicle velocity

  volatile bool updateFlag;  // updateFlag is used to know when to calculate a new RPM

  bool ignoreNextReading;

  WheelState wheelState;

  Wheel(int pinNumber) {
    sensorPin = pinNumber;
    lastReadingMillis = 0;
    currentReadingMillis = 0;
    rpm = 0;
    wheelSpeedMPH = 0;
    updateFlag = false;
    ignoreNextReading = false;
    wheelState = GOOD;

    pinMode(sensorPin, INPUT);
  }


  // Calculates RPM based on elapsed time between last reading and current time
  // Only runs after respective ISR is triggered
  void calculateRPM() {

    if (updateFlag) {

      // If this is true, we were at zero RPM, and we cannot do any calculations with this reading
      // So, just get the current reading and wait for the next reading
      if (ignoreNextReading) {
        ignoreNextReading = false;                          // Reset the flag
        lastReadingMillis = millis();                       // Mark the current time
        Serial.println("Ignoring Revolution (from zero)");  // Print a message stating what happened
        return;                                             // Return to main loop, waiting for an interrupt
      }

      // Otherwise, continue normally

      // Clear the update flag
      updateFlag = false;

      // Update currentReadingMillis with the reading that triggered this condition
      currentReadingMillis = millis();

      // Calculate the new RPM value
      if (currentReadingMillis != lastReadingMillis) {
        rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;
        wheelSpeedMPH = rpm * rpmToMphFactor;
      } else {
        //Serial.print("Avoided Divide-By-Zero error, not updating rpm value");
        return;
      }

      // Set lastReadingMillis to the most recent reading
      lastReadingMillis = currentReadingMillis;
    }
  }

  // Checks to see if a certain period of time has passed since last reading
  // If we surpass that threshold, set the RPM to zero
  void checkZeroRPM() {
    if ((millis() - currentReadingMillis) > zeroTimeoutMS) {

      // Set last reading to millis() so we can bounce back once another reading is detected
      currentReadingMillis = millis();

      // Set a flag so that we know we cannot do any valuable calculations with the next reading
      ignoreNextReading = true;

      // Set MPH to zero
      wheelSpeedMPH = 0;
    }
  }

  // Compares wheel speed to GPS vehicle speed to see if we have wheelspin or skidding
  void checkWheelState() {
    if ((wheelSpeedMPH - vehicleSpeedMPH) > wheelSpinThreshold) {
      wheelState = SPIN;
    } else if ((wheelSpeedMPH - vehicleSpeedMPH) < -wheelSkidThreshold) {
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