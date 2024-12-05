// Class that defines shared variables and functions between the four wheels
class Shock {

public:

  int sensorPin;  // GPIO that sensor is hooked up to
  int reading;  // analog reading value from ESP32
  int position;  // calculated position value (0-100%) from reading

  Shock(int pinNumber) {
    sensorPin = pinNumber;
    int reading = 0;
    int position = 0;
    pinMode(sensorPin, INPUT);
  }


  // Gets analog reading of sensor and calculates position (0-100%)
  void getPosition() {

    reading = analogRead(sensorPin);

    position = map(reading, 0, 4095, 0, 100);

    Serial.print(millis());
    Serial.print(",");
    Serial.println(position);
  }
};