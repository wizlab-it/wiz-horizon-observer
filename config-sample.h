/**
 * @package Wiz Horizon Observer (WHO)
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241014.003
 */

#ifndef CONFIG_H
#define CONFIG_H


/**
 * First level configuration parameters
 * You must change these parameters or WHO won't work
 */

// WiFi credentials
#define WIFI_SSID   "ssid"
#define WIFI_PSK    "psk"

// OpenAI (ChatGPT) parameters
#define _OPENAI_API_KEY   "openai-api-key"


/**
 * Second level configuration parameters
 * You can change these parameters to customize WHO's behaviour
 */

// Camera parameters
#define _CAMERA_FRAMESIZE   FRAMESIZE_XGA
#define _CAMERA_QUALITY     8

// OpenAI (ChatGPT) API
#define _OPENAI_HOSTNAME    "api.openai.com"          // testwhoserver.example.com
#define _OPENAI_ENDPOINT    "/v1/chat/completions"    // /whoServer.php
#define _OPENAI_QUESTION    "What is the probability that there are one or more ships or a boats at the horizon in the image? Please respond only with a digit that could be: 4 if you are totally sure there are boats or ships; 3 if you are quite sure, but not totally sure, that there are boats or ships; 2 if you are quite sure that there are no boats or shipt; 1 if you are totally sure that there are no boats or ships."


/**
 * Third level configuration parameters
 * Probably you won't need to change these parameters to make WHO works fine
 */
// I2C bus parameters
#define _I2C_SDA_GPIO   1
#define _I2C_SCL_GPIO   42
#define _I2C_FREQ       400000

// Servo Motors
#define _SERVO_ROLL_PIN   21
#define _SERVO_PITCH_PIN  20

// Oled parameters
#define _OLED_ADDRESS     0x3C  //OLED I2C Address
#define _OLED_WIDTH       128   //OLED display width, in pixels
#define _OLED_HEIGHT      64    //OLED display height, in pixels
#define _OLED_LOOP_CHARS  "-\\|/"

// MPU-6050 parameters
#define _MPU6050_ADDRESS    0x68

// OpenAI (ChatGPT) parameters
#define _OPENAI_TIMEOUT_CONNECT   10
#define _OPENAI_TIMEOUT_DATA      15
#define _OPENAI_BUFFER_SIZE       5000
#define _OPENAI_PAYLOAD_HEADERS   "POST %s HTTP/1.1\r\nHost: %s\r\nAuthorization: Bearer %s\r\nContent-Length: %d\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"
#define _OPENAI_PAYLOAD_PRE       "{ \"model\":\"gpt-4o\", \"messages\":[{\"role\":\"user\", \"content\":[{ \"type\":\"text\", \"text\":\"" _OPENAI_QUESTION "\" },{ \"type\":\"image_url\", \"image_url\":{ \"url\":\"data:image/jpeg;base64,"
#define _OPENAI_PAYLOAD_POST      "\" }}]}]}"


#endif