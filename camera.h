/**
 * @package Wiz Horizon Observer (WHO)
 * Camera
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241004.007
 */

#ifndef CAMERA_H
#define CAMERA_H


/**
 * Includes
 */
#include <Arduino.h>
#include "esp_camera.h"


/**
 * Defines
 */
//Camera Parameters (ESP32S3 Dev Module - CAMERA_MODEL_ESP32S3_EYE)
#define _CAMERA_PWDN_GPIO_NUM     -1
#define _CAMERA_RESET_GPIO_NUM    -1
#define _CAMERA_XCLK_GPIO_NUM     15
#define _CAMERA_SIOD_GPIO_NUM      4
#define _CAMERA_SIOC_GPIO_NUM      5
#define _CAMERA_Y9_GPIO_NUM       16
#define _CAMERA_Y8_GPIO_NUM       17
#define _CAMERA_Y7_GPIO_NUM       18
#define _CAMERA_Y6_GPIO_NUM       12
#define _CAMERA_Y5_GPIO_NUM       10
#define _CAMERA_Y4_GPIO_NUM        8
#define _CAMERA_Y3_GPIO_NUM        9
#define _CAMERA_Y2_GPIO_NUM       11
#define _CAMERA_VSYNC_GPIO_NUM     6
#define _CAMERA_HREF_GPIO_NUM      7
#define _CAMERA_PCLK_GPIO_NUM     13


/**
 * Camera class
 */
class Camera {
  private:
    framesize_t _frameSize;
    int _jpegQuality;

  public:
    Camera(framesize_t frameSize, int jpegQuality);
    bool init();
    int8_t takePhoto(camera_fb_t **fb);
};


#endif