// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <CAN.h>

// Task to run on second core (dual-core processing)
TaskHandle_t Task1;

const int speedSensorPin = 13;
int targetsPerRevolution = 8;

unsigned long lastReadingMillis = 1;
unsigned long currentReadingMillis = 1;

float rpm = 0;
float rpmBuffer = 0; // this is used as a buffer of last rpm reading to use while sending CAN data. This is in order to prevent data corruption with new values being read into rpm variable

#define n 10  // number of previous readings to use in averaging;
int lastRpmReadings[n] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int rpmArrayIndex = 0;

int timeoutThreshold = 2000;  // Time in ms without a reading before RPM display resets

bool updateFlag = false;

int canSendInterval = 100;
unsigned long lastCanSendTime = 0;




void setup() {
  Serial.begin(115200);


  pinMode(speedSensorPin, INPUT);
  //
  // If the speed sensor detects a metal, it outputs a HIGH
  // Otherwise, it outputs a LOW
  //
  // Thus, we want to trigger interupt on LOW
  //
  attachInterrupt(digitalPinToInterrupt(speedSensorPin), updateRPM, RISING);



  
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  delay(500);


  Serial.println("CAN Sender");

  CAN.setPins(4, 5);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(1000E3)) {
    Serial.println("Starting CAN failed!");
    while (1)
      ;
  }
}

void loop() {


  currentReadingMillis = millis();

  if (updateFlag) {

    // Show that a revolution was detected
    // Serial.println("Revolution detected!");

    // Clear the update flag
    updateFlag = false;

    // Serial.print("RPM: ");
     //Serial.println(rpm);
  }

  // If reached reading timeout
  if ((millis() - lastReadingMillis) > timeoutThreshold) {

    // Set last reading to millis()
    lastReadingMillis = millis();

    // Set rpm to zero
    rpm = 0;
    updateFlag = true;
  }

}

void updateRPM() {

  // Calculate the new RPM value
  rpm = (1.00 / (float(currentReadingMillis - lastReadingMillis) / 1000.0)) * 60.0 / targetsPerRevolution;

  // Set lastReadingMillis to the most recent reading
  lastReadingMillis = currentReadingMillis;

  updateFlag = true;
}


void updateCanbus() {


  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  //Serial.print("Sending packet ... ");
  
  rpmBuffer = rpm;

  CAN.beginPacket(0x96);
  CAN.print((int)rpmBuffer);
  CAN.endPacket();

  //Serial.println("done");

}



// Task 1 executes on secondary core of ESP32 and solely looks at the secondary of the CVT
// All other processing is done on primary core
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

if ((millis() - lastCanSendTime) > canSendInterval) {
  updateCanbus();
  lastCanSendTime = millis();
}


  }
}
