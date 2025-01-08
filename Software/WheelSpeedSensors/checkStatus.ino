void checkStatus() {

/*
  Status flag bits for WheelSpeedSensors
  
  Bit 0 (LSB): Front Left Wheel
  Bit 1: Front Left Travel Sensor
  Bit 2: Front Right Wheel
  Bit 3: Front Right Travel Sensor
  Bit 4: Rear Left Wheel
  Bit 5: Rear Left Travel Sensor
  Bit 6: Rear Right Wheel
  Bit 7 (MSB): Rear Right Travel Sensor
*/

  // Ternary operator is used for briefness

  // bitWrite(x, n, b)
  // x: the numeric variable to which to write
  // n: which bit of the number to write, starting at 0 for the least-significant (rightmost) bit
  // b: the value to write to the bit (0 or 1)

  // To detect if wheel speed sensor is not working, look at GPS speed. If wheel speed sensor is reading
  // zero and GPS speed is above a certain threshold, then we know the sensor is not working. We
  // may also be able to reference secondary RPM for this too, if that logic works
  int minGpsSpeed = 1;

  bitWrite(statusWheels, 0, ((vehicleSpeedMPH > minGpsSpeed && frontLeftWheel.rpm == 0) ? 0 : 1));
  bitWrite(statusWheels, 1, ((frontLeftShock.reading == 0 || frontLeftShock.reading == 4095) ? 0 : 1));  // If shock reads min or max, we have an issue
  bitWrite(statusWheels, 2, ((vehicleSpeedMPH > minGpsSpeed && frontRightWheel.rpm == 0) ? 0 : 1));
  bitWrite(statusWheels, 3, ((frontRightShock.reading == 0 || frontRightShock.reading == 4095) ? 0 : 1));  // If shock reads min or max, we have an issue
  bitWrite(statusWheels, 4, ((vehicleSpeedMPH > minGpsSpeed && rearLeftWheel.rpm == 0) ? 0 : 1));
  bitWrite(statusWheels, 5, ((rearLeftShock.reading == 0 || rearLeftShock.reading == 4095) ? 0 : 1));  // If shock reads min or max, we have an issue
  bitWrite(statusWheels, 6, ((vehicleSpeedMPH > minGpsSpeed && rearRightWheel.rpm == 0) ? 0 : 1));
  bitWrite(statusWheels, 7, ((rearRightShock.reading == 0 || rearRightShock.reading == 4095) ? 0 : 1));  // If shock reads min or max, we have an issue
}