/**
 * @package Wiz Horizon Observer (WHO)
 * Inertial Measurement Unit (IMU) based on MPU-5060 and Kalman filter
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241003.005
 */

#include "IMU.h"


/**
 * Initialize variables
 */
uint32_t IMU::lastProcessed = 0;
uint8_t IMU::mpuAddress;
int16_t IMU::accelX, IMU::accelY, IMU::accelZ;
int16_t IMU::gyroX,  IMU::gyroY,  IMU::gyroZ;
double IMU::kalXAngle, IMU::kalYAngle;


/**
 * Kalman initialization function definition
 */
inline void Kalman_Init(Kalman *kalPointer) {
  /* We will set the variables like so, these can also be tuned by the user */
  kalPointer->Q_angle = _KALMAN_Q_ANGLE;
  kalPointer->Q_bias = _KALMAN_Q_BIAS;
  kalPointer->R_measure = _KALMAN_R_MEASURE;

  kalPointer->angle = 0; // Reset the angle
  kalPointer->bias = 0;  // Reset bias

  kalPointer->P[0][0] = 0; // Since we assume that the bias is 0 and we know the starting angle (use setAngle), the error covariance matrix is set like so - see: http://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical
  kalPointer->P[0][1] = 0;
  kalPointer->P[1][0] = 0;
  kalPointer->P[1][1] = 0;
}


/**
 * Kalman "get angle" function definition
 * @param The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
 */
inline double Kalman_GetAngle(Kalman *kalPointer, double newAngle, double newRate, double dt) {
  // KasBot V2  -  Kalman filter module - http://www.x-firm.com/?page_id=145
  // Discrete Kalman filter time update equations - Time Update ("Predict")
  // Update xhat - Project the state ahead
  /* Step 1 */
  kalPointer->rate = newRate - kalPointer->bias;
  kalPointer->angle += dt * kalPointer->rate;

  // Update estimation error covariance - Project the error covariance ahead
  /* Step 2 */
  kalPointer->P[0][0] += dt * (dt * kalPointer->P[1][1] - kalPointer->P[0][1] - kalPointer->P[1][0] + kalPointer->Q_angle);
  kalPointer->P[0][1] -= dt * kalPointer->P[1][1];
  kalPointer->P[1][0] -= dt * kalPointer->P[1][1];
  kalPointer->P[1][1] += kalPointer->Q_bias * dt;

  // Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
  // Calculate Kalman gain - Compute the Kalman gain
  /* Step 4 */
  kalPointer->S = kalPointer->P[0][0] + kalPointer->R_measure;
  /* Step 5 */
  kalPointer->K[0] = kalPointer->P[0][0] / kalPointer->S;
  kalPointer->K[1] = kalPointer->P[1][0] / kalPointer->S;

  // Calculate angle and bias - Update estimate with measurement zk (newAngle)
  /* Step 3 */
  kalPointer->y = newAngle - kalPointer->angle;
  /* Step 6 */
  kalPointer->angle += kalPointer->K[0] * kalPointer->y;
  kalPointer->bias += kalPointer->K[1] * kalPointer->y;

  // Calculate estimation error covariance - Update the error covariance
  /* Step 7 */
  kalPointer->P[0][0] -= kalPointer->K[0] * kalPointer->P[0][0];
  kalPointer->P[0][1] -= kalPointer->K[0] * kalPointer->P[0][1];
  kalPointer->P[1][0] -= kalPointer->K[1] * kalPointer->P[0][0];
  kalPointer->P[1][1] -= kalPointer->K[1] * kalPointer->P[0][1];

  return kalPointer->angle;
};


/**
 * IMU::init() function definition
 */
void IMU::init(uint8_t i2cAddress) {
  mpuAddress = i2cAddress;

  Wire.beginTransmission(mpuAddress);
  Wire.write(_MPU_REG);
  Wire.write(7);
  for(byte i = 0; i < 3; i++) {
    Wire.write(0);
  }
  Wire.endTransmission(false);

  Wire.beginTransmission(mpuAddress);
  Wire.write(_MPU_PWR_MGMT_1);
  Wire.write(0x01);
  Wire.endTransmission(true);

  delay(100);

  Kalman_Init(&kalmanX);
  Kalman_Init(&kalmanY);

  MPU6050Read();

  double roll, pitch;
  IMU::RollPitchFromAccel(&roll, &pitch);

  kalmanX.angle = roll; // Set starting angle
  kalmanY.angle = pitch;
  gyroXAngle = roll;
  gyroYAngle = pitch;

  lastProcessed = micros();
}


/**
 * IMU::read() function definition
 */
void IMU::read() {
  static double dt = 0;

  MPU6050Read();

  dt = (double)(micros() - lastProcessed) / 1000000;
  lastProcessed = micros();

  double roll, pitch;
  IMU::RollPitchFromAccel(&roll, &pitch);

  double gyroXRate, gyroYRate;
  gyroXRate = (double)gyroX / 131.0; // Convert to deg/s
  gyroYRate = (double)gyroY / 131.0; // Convert to deg/s

  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if((roll < -90 && kalXAngle > 90) || (roll > 90 && kalXAngle < -90)) {
    kalmanX.angle = roll;
    kalXAngle = roll;
    gyroXAngle = roll;
  } else {
    kalXAngle = Kalman_GetAngle(&kalmanX, roll, gyroXRate, dt); // Calculate the angle using a Kalman filter
  }

  if (abs(kalXAngle) > 90) gyroYRate = -gyroYRate; // Invert rate, so it fits the restriced accelerometer reading
  kalYAngle = Kalman_GetAngle(&kalmanY, pitch, gyroYRate, dt);

  //gyroXAngle += gyroXRate * dt; // Calculate gyro angle without any filter
  //gyroYAngle += gyroYRate * dt;
  gyroXAngle += kalmanX.rate * dt; // Calculate gyro angle using the unbiased rate
  gyroYAngle += kalmanY.rate * dt;

  // Reset the gyro angle when it has drifted too much
  if(gyroXAngle < -180 || gyroXAngle > 180) gyroXAngle = kalXAngle;
  if(gyroYAngle < -180 || gyroYAngle > 180) gyroYAngle = kalYAngle;
}


/**
 * IMU class support functions
 */
uint32_t IMU::getLastReadTime() {
  return lastProcessed;
}
int16_t IMU::getRawAccelX() {
  return accelX;
}
int16_t IMU::getRawAccelY() {
  return accelY;
}
int16_t IMU::getRawAccelZ() {
  return accelZ;
}
int16_t IMU::getRawGyroX() {
  return gyroX;
}
int16_t IMU::getRawGyroY() {
  return gyroY;
}
int16_t IMU::getRawGyroZ() {
  return gyroZ;
}
double IMU::getRoll() {
  return kalXAngle;
}
double IMU::getPitch() {
  return kalYAngle;
}


/**
 * IMU::MPU6050Read() function definition
 */
void IMU::MPU6050Read() {
  Wire.beginTransmission(mpuAddress);
  Wire.write(_MPU_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(mpuAddress, 14, true);

  accelX = (int16_t)(Wire.read() << 8 | Wire.read());
  accelY = (int16_t)(Wire.read() << 8 | Wire.read());
  accelZ = (int16_t)(Wire.read() << 8 | Wire.read());
  Wire.read();
  Wire.read(); // Temperature
  gyroX = (int16_t)(Wire.read() << 8 | Wire.read());
  gyroY = (int16_t)(Wire.read() << 8 | Wire.read());
  gyroZ = (int16_t)(Wire.read() << 8 | Wire.read());
}


/**
 * IMU::RollPitchFromAccel() function definition
 */
void IMU::RollPitchFromAccel(double *roll, double *pitch) {
  *roll = atan2((double)accelY, (double)accelZ) * RAD_TO_DEG;
  *pitch = atan((double)-accelX / hypotenuse((double)accelY, (double)accelZ)) * RAD_TO_DEG;
}