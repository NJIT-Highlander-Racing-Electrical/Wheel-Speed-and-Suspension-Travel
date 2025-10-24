const float wheelDiameter = 23;      // Diameter of our wheels in inches
const int targetsPerRevolution = 4;  // number of sensing points per revolution on the wheel
const float wheelSpinThreshold = 5;  // Speed difference (mph) above GPS vehicle velocity where we will declare wheelspin
const float wheelSkidThreshold = 5;  // Speed difference (mph) below GPS vehicle velocity where we will declare skidding

const float rpmToMphFactor = wheelDiameter / 63360.0 * 3.1415 * 60.0;  // When wheel RPM is multiplied by this, it results in that wheel's linear speed in MPH

float vehicleSpeedMPH = 0;  // Since GPS velocity is given in m/s, this converts and stores to MPH

// Minimum time between valid readings (microseconds) - prevents noise/bouncing
// 5000us = 5ms = maximum 720 RPM (well above expected max)
const unsigned long MIN_PULSE_INTERVAL = 5000;

// Maximum time to wait before declaring zero RPM
// At 1 MPH with 4 targets, we get a pulse every ~1.03 seconds
// Allow 2 seconds (2,000,000 microseconds) before declaring zero
const unsigned long ZERO_TIMEOUT_MICROS = 2000000;

enum WheelState {
  GOOD,
  SPIN,
  SKID
};

// Class that defines shared variables and functions between the four wheels
class Wheel {

public:

  int sensorPin;  // GPIO that sensor is hooked up to

  volatile unsigned long lastReadingMicros;
  volatile unsigned long currentReadingMicros;
  volatile bool updateFlag;

  unsigned long nextExpectedMicros;
  
  float rpm;  // variable to store calculated RPM value
  float wheelSpeedMPH;  // calculated wheel velocity for comparison with GPS vehicle velocity

  bool ignoreNextReading;
  bool isFirstReading;

  WheelState wheelState;

  // Moving average for stability
  static const int AVG_SAMPLES = 3;
  float rpmHistory[AVG_SAMPLES];
  int rpmHistoryIndex;
  int rpmHistoryCount;  // Track how many samples we have

  Wheel(int pinNumber) {
    sensorPin = pinNumber;
    unsigned long currentTime = micros();
    lastReadingMicros = currentTime;
    currentReadingMicros = currentTime;
    nextExpectedMicros = currentTime + ZERO_TIMEOUT_MICROS;
    rpm = 0;
    wheelSpeedMPH = 0;
    updateFlag = false;
    ignoreNextReading = false;
    isFirstReading = true;
    wheelState = GOOD;
    rpmHistoryIndex = 0;
    rpmHistoryCount = 0;

    // Initialize RPM history to zero
    for(int i = 0; i < AVG_SAMPLES; i++) {
      rpmHistory[i] = 0;
    }

    pinMode(sensorPin, INPUT);
  }

  // Calculates RPM based on elapsed time between last reading and current time
  // Only runs after respective ISR is triggered
  void calculateRPM() {
    if (!updateFlag) return;

    // Capture timing variables atomically to avoid race conditions
    noInterrupts();
    unsigned long capturedCurrent = currentReadingMicros;
    unsigned long capturedLast = lastReadingMicros;
    updateFlag = false;
    interrupts();

    // Skip first reading - need two points to calculate speed
    if (isFirstReading) {
      noInterrupts();
      lastReadingMicros = capturedCurrent;
      interrupts();
      isFirstReading = false;
      nextExpectedMicros = capturedCurrent + ZERO_TIMEOUT_MICROS;
      return;
    }

    // If we just recovered from zero, ignore this reading (re-establish baseline)
    if (ignoreNextReading) {
      noInterrupts();
      lastReadingMicros = capturedCurrent;
      interrupts();
      ignoreNextReading = false;
      nextExpectedMicros = capturedCurrent + ZERO_TIMEOUT_MICROS;
      return;
    }

    // Calculate time difference (handles overflow correctly since both are unsigned long)
    unsigned long timeDifference = capturedCurrent - capturedLast;
    
    // Sanity check: reject readings that are too fast (noise/bounce filter)
    if (timeDifference < MIN_PULSE_INTERVAL) {
      return;
    }
    
    // Sanity check: reject readings that are impossibly slow (missed timeout somehow)
    if (timeDifference > ZERO_TIMEOUT_MICROS) {
      noInterrupts();
      lastReadingMicros = capturedCurrent;
      interrupts();
      ignoreNextReading = true;
      return;
    }
    
    // Calculate RPM from time difference
    float instantRPM = (1.00 / (float(timeDifference) / 1000000.0)) * 60.0 / targetsPerRevolution;
    
    // Sanity check: 650 RPM = ~45 MPH, reasonable maximum
    if (instantRPM > 650) {
      Serial.print("RPM over 650 rejected: ");
      Serial.print(instantRPM);
      Serial.print(" on pin ");
      Serial.println(sensorPin);
      return;
    }
    
    // Add to moving average buffer
    rpmHistory[rpmHistoryIndex] = instantRPM;
    rpmHistoryIndex = (rpmHistoryIndex + 1) % AVG_SAMPLES;
    if (rpmHistoryCount < AVG_SAMPLES) {
      rpmHistoryCount++;
    }
    
    // Calculate average RPM
    float sum = 0;
    for(int i = 0; i < rpmHistoryCount; i++) {
      sum += rpmHistory[i];
    }
    rpm = sum / rpmHistoryCount;
    
    // Convert to MPH
    wheelSpeedMPH = rpm * rpmToMphFactor;
    
    // Update timing for next expected reading (with 2x buffer for timeout detection)
    nextExpectedMicros = capturedCurrent + (timeDifference * 2);
    
    // Update last reading time atomically
    noInterrupts();
    lastReadingMicros = capturedCurrent;
    interrupts();
  }

  // Checks to see if a certain period of time has passed since last reading
  // If we surpass that threshold, set the RPM to zero
  void checkZeroRPM() {
    // Don't check if we haven't established baseline yet
    if (isFirstReading || ignoreNextReading) return;
    
    unsigned long currentTime = micros();
    
    // Check if we've exceeded the expected next reading time
    // This handles overflow correctly: if currentTime wraps around,
    // the subtraction still works due to unsigned arithmetic properties
    unsigned long timeSinceLastReading = currentTime - lastReadingMicros;
    
    if (timeSinceLastReading > ZERO_TIMEOUT_MICROS) {
      // No readings in too long - wheel has stopped
      
      // Reset all state atomically
      noInterrupts();
      lastReadingMicros = currentTime;
      interrupts();
      
      ignoreNextReading = true;  // Next reading will be used to re-establish baseline
      rpm = 0;
      wheelSpeedMPH = 0;
      nextExpectedMicros = currentTime + ZERO_TIMEOUT_MICROS;
      
      // Clear RPM history
      for(int i = 0; i < AVG_SAMPLES; i++) {
        rpmHistory[i] = 0;
      }
      rpmHistoryIndex = 0;
      rpmHistoryCount = 0;
    }
  }

  // Compares wheel speed to GPS vehicle speed to see if we have wheelspin or skidding
  void checkWheelState() {
    // Only check wheel state if we have valid GPS data
    if (vehicleSpeedMPH < 0.1) {
      wheelState = GOOD;  // Don't declare spin/skid at very low speeds
      return;
    }
    
    float speedDifference = wheelSpeedMPH - vehicleSpeedMPH;
    
    if (speedDifference > wheelSpinThreshold) {
      wheelState = SPIN;
    } else if (speedDifference < -wheelSkidThreshold) {
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
  
  // Call this from ISR - handles debouncing
  void handleInterrupt() {
    unsigned long now = micros();
    
    // Debounce: ignore triggers that are too close together
    // This prevents double-triggers from noise/bouncing
    unsigned long timeSinceLast = now - currentReadingMicros;
    if (timeSinceLast < MIN_PULSE_INTERVAL) {
      return;  // Too fast, likely bounce/noise
    }
    
    // Update timestamps
    lastReadingMicros = currentReadingMicros;
    currentReadingMicros = now;
    
    // Signal main loop to calculate
    updateFlag = true;
  }
};