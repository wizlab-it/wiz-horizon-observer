/**
 * @package Wiz Horizon Observer (WHO)
 * Inertial Measurement Unit (IMU) based on MPU-5060 and Kalman filter
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241003.004
 */

#ifndef __IMU_H_
#define __IMU_H_


/**
 * Includes
 */
#include <Arduino.h>
#include <Wire.h>


/**
 * Defines
 */
// Kalman configurations
#define _KALMAN_Q_ANGLE     0.001   // Noise variance for the accelerometer
#define _KALMAN_Q_BIAS      0.003   // Noise variance for the gyro bias
#define _KALMAN_R_MEASURE   0.03    // Measurement noise variance - variance of the measurement noise

// Constants and macros
#define RAD_TO_DEG 180.0 / M_PI
#define sqr(x) x * x
#define hypotenuse(x, y) sqrt(sqr(x) + sqr(y))

// MPU-6050
#define _MPU_ACCEL_XOUT_H 0x3B
#define _MPU_REG 0x19
#define _MPU_PWR_MGMT_1 0x6B


/**
 * Kalman structure
 */
typedef struct kalman_t {
  double Q_angle;   // Process noise variance for the accelerometer
  double Q_bias;    // Process noise variance for the gyro bias
  double R_measure; // Measurement noise variance - this is actually the variance of the measurement noise

  double angle; // The angle calculated by the Kalman filter - part of the 2x1 state vector
  double bias;  // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
  double rate;  // Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate

  double P[2][2]; // Error covariance matrix - This is a 2x2 matrix
  double K[2];    // Kalman gain - This is a 2x1 vector
  double y;       // Angle difference
  double S;       // Estimate error
} Kalman;


/**
 * Variables
 */
static Kalman kalmanX;
static Kalman kalmanY;
static double gyroXAngle, gyroYAngle; // Angle calculate using the gyro only


/**
 * IMU class
 */
class IMU {
  public:
    static void init(uint8_t i2cAddress);
    static void read();
    static uint32_t getLastReadTime();
    static int16_t getRawAccelX();
    static int16_t getRawAccelY();
    static int16_t getRawAccelZ();
    static int16_t getRawGyroX();
    static int16_t getRawGyroY();
    static int16_t getRawGyroZ();
    static double getRoll();
    static double getPitch();

  private:
    static uint8_t mpuAddress;
    static uint32_t lastProcessed;
    static int16_t accelX, accelY, accelZ;
    static int16_t gyroX, gyroY, gyroZ;
    static double kalXAngle, kalYAngle;

    static void MPU6050Read();
    static void RollPitchFromAccel(double *roll, double *pitch);
};


#endif