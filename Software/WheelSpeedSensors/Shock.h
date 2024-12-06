/*

  We would like to report wheel displacement using these LPPS-22-200 Linear Travel Sensors.
  This involves experimentally finding the analog readings we would see at fully compressed
  and fully extended. It is also good to have distance from heim to heim so that the
  suspension team has a dimensional reference. Using that reference, they can provide a
  lookup table that relates heim-to-heim distance to wheel displacement. Then, we can use
  that lookup table in this code to return the wheel displacement based on analog readings.  

*/

#include "ShockLookupTable.h"

const float compressedLength = 12;  // Length from heim-to-heim when fully compressed
const float restLength = 15;        // Length from heim-to-heim when at resting height
const float extendedLength = 20;    // Length from heim-to-heim when fully extended

const int compressedReading = 4095;  // Reading from sensor when fully compressed
const int restReading = 2048;        // Reading from sensor when at resting height
const int extendedReading = 0;       // Reading from sensor when fully extended


// Class that defines shared variables and functions between the four wheels
class Shock {

public:

  int sensorPin;   // GPIO that sensor is hooked up to
  int reading;     // analog reading value from ESP32
  float length;    // length of the linear sensor from heim-to-heim
  float distance;  // wheel displacement +/- from zero point in inches

  Shock(int pinNumber) {
    sensorPin = pinNumber;
    int reading = 0;
    float length = 0;
    float distance = 0;
    pinMode(sensorPin, INPUT);
  }


  void getPosition() {

    // Get initial analog reading
    reading = analogRead(sensorPin);

    // Converts analog value into heim-to-heim length
    length = mapfloat(reading, compressedReading, extendedReading, compressedLength, extendedLength);

    // Maps heim-to-heim length to wheel displacement
    // Works in 0.1" increments, so multiply the float by ten and cast to an int (e.g. 3.1" becomes int 31)
    distance = lengthToDisplacement((int)(length * 10));

    Serial.print(millis());
    Serial.print(",");
    Serial.println(reading);
  }

  // Function for float mapping
  float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  float lengthToDisplacement(int scaledLength) {

    // We are passed in a lengthVal integer which represents the heim-to-heim length times 10
    // This allows us to have 0.1" increments that we can easily put into an array as integers

    // Check if the scaled length exists in our lookup table
    auto it = wheelDisplacementMap.find(scaledLength);
    if (it != wheelDisplacementMap.end()) {
      return it->second;  // Return the corresponding wheel displacement
    } else {
      return 0.0;  // If we could not find it, just return 0.0
    }
  }
};
