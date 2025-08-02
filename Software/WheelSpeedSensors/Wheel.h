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
  unsigned long nextExpectedMillis;

  float rpm;  // variable to store calculated RPM value

  float wheelSpeedMPH;  // calculated wheel velocity for comparison with GPS vehicle velocity

  volatile bool updateFlag;  // updateFlag is used to know when to calculate a new RPM

  bool ignoreNextReading;

  WheelState wheelState;

  Wheel(int pinNumber) {
    sensorPin = pinNumber;
    unsigned long currentTime = millis();
    lastReadingMillis = currentTime;
    currentReadingMillis = currentTime;
    nextExpectedMillis = currentTime + 3600000; // Set far in future initially (one hour)
    rpm = 0;
    wheelSpeedMPH = 0;
    updateFlag = false;
    ignoreNextReading = true;  // Ignore first reading to establish baseline
    wheelState = GOOD;

    pinMode(sensorPin, INPUT);
  }

  // Calculates RPM based on elapsed time between last reading and current time
  // Only runs after respective ISR is triggered
  void calculateRPM() {

    if (updateFlag) {

      // Clear the update flag first
      updateFlag = false;

      // Update currentReadingMillis with the reading that triggered this condition
      currentReadingMillis = millis();

      // If we're ignoring this reading (first one after timeout), just update timing and return
      if (ignoreNextReading) {
        lastReadingMillis = currentReadingMillis;
        ignoreNextReading = false;
        // Set next expected time far in future until we get a second reading
        nextExpectedMillis = currentReadingMillis + 10000;
        return;
      }

      // Calculate the new RPM value
      unsigned long timeDifference = currentReadingMillis - lastReadingMillis;
      
      if (timeDifference > 0) {
        rpm = (1.00 / (float(timeDifference) / 1000.0)) * 60.0 / targetsPerRevolution;
        
        if (rpm > 650) {
          // 650 RPM comes out to roughly 45 MPH which is more than we'd ever expect to see
          Serial.print("RPM over 650 error: ");
          Serial.println(rpm);
          lastReadingMillis = currentReadingMillis;
          return;
        }
        
        wheelSpeedMPH = rpm * rpmToMphFactor;
        
        // Set next expected reading time with 1.5x buffer
        float f_timeDifference = timeDifference;
        nextExpectedMillis = currentReadingMillis + (unsigned long)(1.5 * f_timeDifference);
        
      } else {
        Serial.println("Avoided Divide-By-Zero error, not updating rpm value");
        return;
      }

      // Set lastReadingMillis to the most recent reading
      lastReadingMillis = currentReadingMillis;
    }
  }

  // Checks to see if a certain period of time has passed since last reading
  // If we surpass that threshold, set the RPM to zero
  void checkZeroRPM() {
    // Only check for zero if we have established a baseline (not ignoring readings)
    if (!ignoreNextReading && millis() > nextExpectedMillis) {

      // Set last reading to current time
      lastReadingMillis = millis();

      // Set a flag so that we know we cannot do valuable calculations with the next reading
      ignoreNextReading = true;

      // Set RPM and MPH to zero
      rpm = 0;
      wheelSpeedMPH = 0;
      
      // Reset next expected time to far future
      nextExpectedMillis = millis() + 10000;
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