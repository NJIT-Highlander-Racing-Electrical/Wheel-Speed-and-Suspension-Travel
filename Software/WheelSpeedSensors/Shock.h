const int readingAtMin = 4095;     // Reading from sensor when fully compressed
const int readingAtMax = 0;        // Reading from sensor when fully extended
const int readingAtCenter = 2048;  // Reading from sensor at center resting point

const float inchesAtMin = -5;  // Displacement of wheels from center when fully compressed (positive)
const float inchesAtMax = +5;  // Displacement of wheels from center when fully extended (negative)


// Class that defines shared variables and functions between the four wheels
class Shock {

public:

  int sensorPin;       // GPIO that sensor is hooked up to
  int reading;         // analog reading value from ESP32
  float displacement;  // wheel displacement +/- from zero point in inches

  Shock(int pinNumber) {
    sensorPin = pinNumber;
    int reading = 0;
    float displacement = 0;
    pinMode(sensorPin, INPUT);
  }


  // Gets analog reading of sensor and calculates position (0-100%)
  void getPosition() {

    reading = analogRead(sensorPin);

    if (reading > readingAtCenter) {
      displacement = mapfloat(reading, readingAtCenter, readingAtMin, 0, inchesAtMin);  // We have positive displacement
    } else {
      displacement = mapfloat(reading, readingAtCenter, readingAtMax, 0, inchesAtMax);  // We have negative displacement
    }

    Serial.print(millis());
    Serial.print(",");
    Serial.println(displacement);
  }
};

// Function for float mapping
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}