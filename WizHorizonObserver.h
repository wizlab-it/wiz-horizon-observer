/**
 * @package Wiz Horizon Observer (WHO)
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241014.028
 */

#ifndef WIZHORIZONOBSERVER_H
#define WIZHORIZONOBSERVER_H


/**
 * Defines
 */
// Name and version
#define APP_NAME "Wiz Horizon Observer"
#define APP_SPLASH_NAME "Wiz\nHorizon\nObserver"
#define APP_SPLASH_VERSION "v0.6"


/**
 * Includes
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <arduino_base64.hpp>
#include "config.h"
#include "IMU.h"
#include "camera.h"


/**
 * Structs
 */
//Vision
struct {
  int rgbLedTimeout = -1;
  int statusTextTimeout = -1;
  camera_fb_t *photo;
  bool processFlag = false;
  uint16_t oledCanary = 0;
} __VISION;


/**
 * Functions
 */
void core0Loop(void *pvParameters);
void processVision();
void oledInitLayout();
void oledPrint(String text, int16_t posX, int16_t posY, uint8_t sizeW, uint8_t sizeH, bool flush);
void statusText(String text, uint8_t seconds);
void setRGB(uint8_t red, uint8_t green, uint8_t blue, uint16_t milliseconds);
int16_t openAISendHTTPRequest(uint8_t* payload, long payloadLength, char* responseToReturn, size_t maxResponseLength);


#endif