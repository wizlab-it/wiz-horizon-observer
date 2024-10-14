/**
 * @package Wiz Horizon Observer (WHO)
 * Camera handling
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241004.011
 */

#include "camera.h"


/**
 * Camera class constructor
 * @param frameSize       Size of the photo (see framesize_t)
 * @param jpegQuality     JPG quality (1-100, low value is better quality)
 */
Camera::Camera(framesize_t frameSize, int jpegQuality) {
  _frameSize = frameSize;
  _jpegQuality = jpegQuality;
}


/**
 * Camera::init
 * Camera initialization and configuration
 * @return    true if camera is successfully initialized; false otherwise
 */
bool Camera::init() {
  //Configure camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = _CAMERA_Y2_GPIO_NUM;
  config.pin_d1 = _CAMERA_Y3_GPIO_NUM;
  config.pin_d2 = _CAMERA_Y4_GPIO_NUM;
  config.pin_d3 = _CAMERA_Y5_GPIO_NUM;
  config.pin_d4 = _CAMERA_Y6_GPIO_NUM;
  config.pin_d5 = _CAMERA_Y7_GPIO_NUM;
  config.pin_d6 = _CAMERA_Y8_GPIO_NUM;
  config.pin_d7 = _CAMERA_Y9_GPIO_NUM;
  config.pin_xclk = _CAMERA_XCLK_GPIO_NUM;
  config.pin_pclk = _CAMERA_PCLK_GPIO_NUM;
  config.pin_vsync = _CAMERA_VSYNC_GPIO_NUM;
  config.pin_href = _CAMERA_HREF_GPIO_NUM;
  config.pin_sscb_sda = _CAMERA_SIOD_GPIO_NUM;
  config.pin_sscb_scl = _CAMERA_SIOC_GPIO_NUM;
  config.pin_pwdn = _CAMERA_PWDN_GPIO_NUM;
  config.pin_reset = _CAMERA_RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.frame_size = _frameSize;
  config.jpeg_quality = _jpegQuality;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count = 1;

  //Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if(err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }

  //Configure sensor
  sensor_t * s = esp_camera_sensor_get();
  s->set_hmirror(s, 1); // mirror
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, -1); // lower the saturation

  //If here, all good
  return true;
}


/**
 * Camera::takePhoto
 * Takes a photo
 * @param **fb    pointer to a camera_fb_t object (frame buffer) where to store the captured image
 * @return        photo capture result (0: success; -1: photo capture failed; -2: photo size is 0)
 */
int8_t Camera::takePhoto(camera_fb_t **fb) {
  // If *fb already contains data, then clear it
  if(*fb) esp_camera_fb_return(*fb);

  //Dispose first picture
  *fb = NULL;
  *fb = esp_camera_fb_get();
  esp_camera_fb_return(*fb);

  //Takes a photo
  *fb = NULL;
  *fb = esp_camera_fb_get();

  //Check if photo capture failed
  if(!(*fb)) return -1;
  if((*fb)->len == 0) return -2;
  return 0;
}