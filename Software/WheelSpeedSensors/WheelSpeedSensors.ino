#include "src/libraries/BajaCAN.h" // https://arduino.github.io/arduino-cli/0.35/sketch-specification/#src-subfolder

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


const int speedSensorPin = 13;
int targetsPerRevolution = 1;

unsigned long lastReadingMillis = 1;
unsigned long currentReadingMillis = 1;

float rpm = 0;

#define n 10 // number of previous readings to use in averaging;
int lastRpmReadings[n] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int rpmArrayIndex = 0;

int timeoutThreshold = 2000;  // Time in ms without a reading before RPM display resets

bool updateFlag = false;




void setup() {
  setupCAN(WHEEL_SPEED);
  Serial.begin(115200);

  pinMode(speedSensorPin, INPUT);
  //
  // If the speed sensor detects a metal, it outputs a HIGH
  // Otherwise, it outputs a LOW
  //
  // Thus, we want to trigger interupt on LOW
  //
  attachInterrupt(digitalPinToInterrupt(speedSensorPin), updateRPM, FALLING);


  while (!Serial);

  Serial.println("CAN Sender");

  CAN.setPins(4,5);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(1000E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {


 if (updateFlag) {

    // Show that a revolution was detected
    Serial.println("Revolution detected!");  

    // Clear the update flag
    updateFlag = false;
    
    Serial.print("RPM: ");
    Serial.println(rpm);

    updateCanbus();

  }

  // If reached reading timeout
  if ((millis() - currentReadingMillis) > timeoutThreshold) {

    // Set last reading to millis()
    currentReadingMillis = millis();

    // Set rpm to zero
    rpm = 0;
    updateFlag = true;
  }

    // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");

  CAN.beginPacket(0x96);
  CAN.print((int)rpm);
  CAN.endPacket();

  Serial.println("done");

  delay(100);


}

void updateRPM() {

  // Set lastReadingMillis to the most recent reading
  lastReadingMillis = currentReadingMillis;

  // Then, update currentReadingMillis with the reading that triggered this condition
  currentReadingMillis = millis();

  // Calculate the new RPM value
  rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;

/*

  // FRAMEWORK FOR AVERAGING LAST 10 VALUES:

  // update last readings array
  lastRpmReadings[rpmArrayIndex] = rpm;
  if (rpmArrayIndex == (n-1)) {
    rpmArrayIndex = 0;
  } else {
    rpmArrayIndex++;
  }

  // Calculate average of new set of n values in array

  // Also need to change the variable printed to display in display update function

*/

  updateFlag = true;
}


void updateCanbus() {
  
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");

  CAN.beginPacket(0x96);
  CAN.print((int)rpm);
  CAN.endPacket();

  Serial.println("done");

  delay(10);
}
