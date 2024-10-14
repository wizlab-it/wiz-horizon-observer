/**
 * @package Wiz Horizon Observer (WHO)
 * @author WizLab.it
 * @board ESP32-S3-WROOM-1-N16R8 (ESP32S3 Dev Module, Flash 16MB, Partition 16MB 3MB APP 9.9MB FAT, OPI PSRAM 8MB)
 * @version 20241014.059
 */

#include "WizHorizonObserver.h"


/**
 * Global variables
 * Initialize Telegram communications and Camera
 */
Adafruit_SSD1306 display(_OLED_WIDTH, _OLED_HEIGHT, &Wire, -1);
Camera camera(_CAMERA_FRAMESIZE, _CAMERA_QUALITY);
Servo servoRoll;
Servo servoPitch;
TaskHandle_t taskCore0Loop;


/**
 * IO0 button interrupt function
 */
void IRAM_ATTR io0Interrupt() {
  __VISION.processFlag = true;
}


/**
 * Setup
 */
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println(F("\n\n\n---\nWiz Horizon Observer - WHO\nby WizLab.it\n---"));
  Serial.println(F("[o] System Initialization:"));

  // Init I2C bus
  Serial.print(F("[*] Init I2C bus: "));
  Wire.begin(_I2C_SDA_GPIO, _I2C_SCL_GPIO, _I2C_FREQ);
  Serial.println(F("OK"));

  // Init display
  Serial.print(F("[*] Init display: "));
  display.begin(SSD1306_SWITCHCAPVCC, _OLED_ADDRESS);
  display.clearDisplay();
  delay(250);
  display.cp437(true); // Use full 256 char 'Code Page 437'
  display.setTextColor(WHITE, BLACK);

  //Splash screen
  display.clearDisplay();
  oledPrint(APP_SPLASH_NAME, 0, 0, 2, 2, false);
  oledPrint(APP_SPLASH_VERSION, 104, 56, 1, 1, true);
  Serial.println(F("OK"));
  delay(2000);

  // Initialization message on display
  display.clearDisplay();
  oledPrint(F("SystemInit\n"), 0, 0, 2, 1, true);

  // Init MPU-6050
  Serial.print(F("[*] Init MPU-6050: "));
  oledPrint(F("MPU-6050: "), -1, -1, 1, 1, true);
  IMU::init(_MPU6050_ADDRESS);
  IMU::read();
  Serial.println(F("OK"));
  oledPrint(F("OK\n"), -1, -1, 1, 1, true);

  // Init servo motors
  Serial.print(F("[*] Init servo motors: "));
  oledPrint(F("Servo motors: "), -1, -1, 1, 1, true);
  servoRoll.attach(_SERVO_ROLL_PIN);
  servoPitch.attach(_SERVO_PITCH_PIN);
  Serial.println(F("OK"));
  oledPrint(F("OK\n"), -1, -1, 1, 1, true);

  // Init RGB led
  Serial.print(F("[*] Init RGB led: "));
  oledPrint(F("RGB Led: "), -1, -1, 1, 1, true);
  pinMode(RGB_BUILTIN, OUTPUT);
  digitalWrite(RGB_BUILTIN, LOW);
  Serial.println(F("OK"));
  oledPrint(F("OK\n"), -1, -1, 1, 1, true);

  // Init WiFi
  Serial.print(F("[*] Init WiFi: "));
  oledPrint(F("WiFi: "), -1, -1, 1, 1, true);
  uint16_t connectingTimer = 0;
  int16_t tmpCurX = display.getCursorX();
  int16_t tmpCurY = display.getCursorY();
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while(WiFi.status() != WL_CONNECTED) {
    // Print connecting animation and delay
    oledPrint(String(_OLED_LOOP_CHARS[connectingTimer % 4]), tmpCurX, tmpCurY, 1, 1, true);

    //If trying to connect for more than 3 minutes, then restart the ESP
    connectingTimer++;
    if(connectingTimer > 1800) {
      Serial.println(F("Restart."));
      oledPrint(F("restart\n"), tmpCurX, tmpCurY, 1, 1, true);
      delay(2500);
      ESP.restart();
    }

    delay(100);
  }
  Serial.println(F("OK"));
  Serial.printf("  [i] Connected to %s\n", WIFI_SSID);
  Serial.printf("  [i] IP address: %s\n", WiFi.localIP().toString().c_str());
  oledPrint(F("OK\n"), tmpCurX, tmpCurY, 1, 1, true);

  // Init camera
  Serial.print(F("[*] Init camera: "));
  oledPrint(F("Camera: "), -1, -1, 1, 1, true);
  if(camera.init()) {
    Serial.println(F("OK"));
    oledPrint(F("OK\n"), -1, -1, 1, 1, true);
  } else {
    Serial.println(F("failed"));
    oledPrint(F("failed\n"), -1, -1, 1, 1, true);
    delay(2500);
    ESP.restart();
  }

  // Init IO0 button
  Serial.print(F("[*] Init IO0 button: "));
  oledPrint(F("IO0 button: "), -1, -1, 1, 1, true);
  pinMode(0, INPUT);
  attachInterrupt(0, io0Interrupt, FALLING);
  Serial.println(F("OK"));
  oledPrint(F("OK\n"), -1, -1, 1, 1, true);

  // Setup complete
  Serial.println(F("[o] Setup complete."));
  Serial.println(F("[o] Running..."));
  oledPrint(F("Done!\n"), -1, -1, 1, 1, true);
  delay(1500);

  // Draw display layout
  oledInitLayout();

  // Start task to process MPU-6050 data running on Core 0
  xTaskCreatePinnedToCore(core0Loop, "TaskCore0Loop", 8192, NULL, 1, &taskCore0Loop, 0);
}


/**
 * Main Loop (runs on Core 1)
 */
void loop() {
  // Process Vision
  processVision();
}


/**
 * core0Loop
 * Loop to run on Core 0
 */
void core0Loop(void *pvParameters) {
  int servoRollAngle, servoPitchAngle;

  while(true) {
    // RGB Led Timeout
    if((__VISION.statusTextTimeout != -1) && (__VISION.statusTextTimeout < millis())) {
      __VISION.statusTextTimeout = -1;
      statusText("", -1);
    }

    // RGB Led Timeout
    if((__VISION.rgbLedTimeout != -1) && (__VISION.rgbLedTimeout < millis())) {
      __VISION.rgbLedTimeout = -1;
      digitalWrite(RGB_BUILTIN, LOW);
    }

    // Read data from MPU-6050 and processes it with the Kalman Filter
    IMU::read();

    // Calcolate roll angle to be set on servo motor
    servoRollAngle = 180.0 - (IMU::getRoll() + 90.0);
    if(servoRollAngle > 135.0) servoRollAngle = 135.0;
    if(servoRollAngle < 45.0) servoRollAngle = 45.0;

    // Calcolate pitch angle to be set on servo motor
    servoPitchAngle = 180.0 - (IMU::getPitch() + 90.0);
    if(servoPitchAngle > 135.0) servoPitchAngle = 135.0;
    if(servoPitchAngle < 45.0) servoPitchAngle = 45.0;

    // Set angles on servo motors
    servoRoll.write(servoRollAngle);
    servoPitch.write(servoPitchAngle);

    // Show angles on display
    char buf[10];
    sprintf(buf, "% 5d", servoRollAngle);
    oledPrint(buf, 3, 18, 2, 2, false);
    sprintf(buf, "% 5d", servoPitchAngle);
    oledPrint(buf, 67, 18, 2, 2, true);
  }
}


/**
 * processVision
 * Process Vision
 */
void processVision() {
  // Check if to process Vision
  if(!__VISION.processFlag) return;
  __VISION.processFlag = false;

  // Clear photo details and transmit on display
  display.fillRect(3, 46, 59, 7, BLACK);
  display.fillRect(67, 46, 58, 7, BLACK);
  display.display();

  // Take photo
  Serial.print(F("[*] Take photo: "));
  statusText(F("Taking photo..."), 10);
  int8_t photoResult = camera.takePhoto(&(__VISION.photo));

  // Check photo result: exits if error
  if(photoResult != 0) {
    Serial.printf("error (%d)\n", photoResult);
    statusText(F("Photo failed"), 10);
    setRGB(0, 0, RGB_BRIGHTNESS, 1000); //Blue
    return;
  }

  // If here, photo has been taken successfully
  Serial.println(F("OK"));
  Serial.printf("  [i] Photo size: %d bytes\n", __VISION.photo->len);
  statusText(F("Photo OK"), 10);

  // Print photo size on display
  uint16_t photoSizeK = __VISION.photo->len / 1024;
  char buf[15];
  sprintf(buf, "% 6d KB", photoSizeK);
  oledPrint(buf, 9, 46, 1, 1, true);

  // Prepare OpenAI payload

  // Calculate payload size and allocate memory
  size_t photoEncodedLength = base64::encodeLength(__VISION.photo->len);
  Serial.printf("  [i] Base64 photo size: %d bytes\n", photoEncodedLength);
  size_t openaiPayloadLen = strlen(_OPENAI_PAYLOAD_PRE) + photoEncodedLength + strlen(_OPENAI_PAYLOAD_POST) + 100;
  char* openaiPayload = (char *)ps_malloc(openaiPayloadLen);
  memset(openaiPayload, 0, openaiPayloadLen);

  // Add payload preamble
  strcpy(openaiPayload, _OPENAI_PAYLOAD_PRE);

  // Base64 encode photo and append to the payload, then free the Camera buffer
  base64::encode(__VISION.photo->buf, __VISION.photo->len, (openaiPayload + strlen(openaiPayload)));
  esp_camera_fb_return(__VISION.photo);

  // Append payload closure and calculate effective payload length
  strcpy(openaiPayload + strlen(openaiPayload), _OPENAI_PAYLOAD_POST);
  openaiPayloadLen = strlen(openaiPayload);

  // OpenAI Payload is ready

  // Send HTTP request to OpenAI
  Serial.println(F("[*] Send request to OpenAI"));
  Serial.printf("  [i] OpenAI Payload size: %d bytes\n", openaiPayloadLen);
  char* openaiResponse = (char *)ps_malloc(_OPENAI_BUFFER_SIZE + 10);
  memset(openaiResponse, 0, (_OPENAI_BUFFER_SIZE + 10));
  int16_t openaiResponseCode = openAISendHTTPRequest((uint8_t*)openaiPayload, openaiPayloadLen, openaiResponse, _OPENAI_BUFFER_SIZE);
  free(openaiPayload);
  Serial.printf("  [i] OpenAI response code: %d\n", openaiResponseCode);

  // If responseCode is <1, then something went wrong. Print error, free response buffer and exit
  if(openaiResponseCode < 1) {
    Serial.println(F("[-] Photo analysis failed"));
    statusText(F("Photo analysis failed"), 10);
    free(openaiResponse);
    return;
  }

  // If here, there's a response from OpenAI, process response

  // Parse JSON data
  JsonDocument json;
  DeserializationError jsonStatus = deserializeJson(json, openaiResponse);
  free(openaiResponse);
  if(jsonStatus != DeserializationError::Ok) {
    Serial.println(F("[-] Error parsing OpenAI JSON response"));
    statusText(F("Invalid response"), 10);
    return;
  }

  // Get probability that a ship on the horizon is in the image
  int shipProbability = 0;
  const char* openaiResponseContent = json["choices"][0]["message"]["content"];
  if(openaiResponseContent != NULL) {
    shipProbability = atoi(openaiResponseContent);
  }
  if((shipProbability == 0) || (shipProbability > 4)) {
    Serial.println(F("[-] Image analysis result is not valid"));
    statusText(F("Invalid analysis"), 10);
  }

  // Show analysis result
  Serial.printf("[+] Image analysis result: %d\n", shipProbability);
  switch(shipProbability) {
    case 4: //4: yes
      setRGB(RGB_BRIGHTNESS, 0, 0, 10000); //Red
      statusText(F("Boat: YES"), 10);
      break;
    case 3: //3: probably yes
      setRGB(RGB_BRIGHTNESS, 0, RGB_BRIGHTNESS, 10000); //Purple
      statusText(F("Boat: probably yes"), 10);
      break;
    case 2: //2: probably no
      setRGB(RGB_BRIGHTNESS, (RGB_BRIGHTNESS / 8), 0, 10000);
      statusText(F("Boat: probably no"), 10);
      break;
    case 1: //1: no
      setRGB(0, RGB_BRIGHTNESS, 0, 10000);
      statusText(F("Boat: no"), 10);
      break;
  }

  // To be sure, reset processVision flag
  __VISION.processFlag = false;
}


/**
 * openAISendHTTPRequest
 * Send HTTP request to OpenAI
 * @param payload             Buffer with data to be sent to OpenAI
 * @param payloadLength       Buffer length
 * @param responseToReturn    Buffer that will be filled with the response from OpenAI
 * @param maxResponseLength   Maximum length of the response from OpenAI
 * @return Status:
 *                  -101: WiFi not connected
 *                  -102: connection timeout
 *                  -103: error sending request headers
 *                  -104: error sending request body
 *                  -105: response is too large
 *                  -106: receive data timeout
 *                  >0: OK, size of received data
 */
int16_t openAISendHTTPRequest(uint8_t* payload, long payloadLength, char* responseToReturn, size_t maxResponseLength) {
  if(WiFi.status() != WL_CONNECTED) return -101;

  // WiFi Client, no certificate check
  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();

  // Connect to remote server
  statusText(F("Open connection..."), 20);
  if(!wifiClient.connect(_OPENAI_HOSTNAME, 443, (_OPENAI_TIMEOUT_CONNECT * 1000))) return -102;

  // Send request header (uses responseToReturn to temporarily store the headers)
  statusText("Sending data...", 60);
  memset(responseToReturn, 0, maxResponseLength);
  sprintf(responseToReturn, _OPENAI_PAYLOAD_HEADERS, _OPENAI_ENDPOINT, _OPENAI_HOSTNAME, _OPENAI_API_KEY, payloadLength);
  size_t requestHeaderSent = wifiClient.write((uint8_t*)responseToReturn, strlen(responseToReturn));
  if(requestHeaderSent == 0) {
    wifiClient.stop();
    return -103;
  }

  // Init progressbar
  display.drawRect(68, 47, 30, 5, WHITE);

  // Send request body
  char oledBuffer[10];
  unsigned long tnxSent = 0, tnxLastSent = 0;
  uint8_t sentPct = 0;
  for(size_t n=0; n<payloadLength; n=n+1024) {
    if((n + 1024) < payloadLength) {
      tnxLastSent = wifiClient.write((payload + n), 1024);
    } else if((payloadLength % 1024) > 0) {
      tnxLastSent = wifiClient.write((payload + n), (payloadLength % 1024));
    }
    if(tnxLastSent == 0) {
      wifiClient.stop();
      return -104;
    }
    tnxSent += tnxLastSent;

    // Print sent data on display and handle progressbar
    sentPct = (tnxSent * 100) / payloadLength;
    display.fillRect(68, 47, ((sentPct * 30) / 100), 5, WHITE);
    sprintf(oledBuffer, "%*d%%", 3, sentPct);
    oledPrint(oledBuffer, 101, 46, 1, 1, true);
  }

  // Request has been sent. Now receive the response

  // Get ready to receive the response
  statusText(F("Receiving data..."), 60);
  uint8_t nlcrCounter = 0;
  bool isHeader = true;
  unsigned long rcvTimeout = millis() + (_OPENAI_TIMEOUT_DATA * 1000);
  size_t responseLength = 0;
  memset(responseToReturn, 0, maxResponseLength);
  char c;

  // Receive data
  while(rcvTimeout > millis()) {
    if(wifiClient.available() > 0) {
      while(wifiClient.available() > 0) {
        c = wifiClient.read();
        if(isHeader) {
          if((c == '\n') || (c == '\r')) nlcrCounter++;
          else nlcrCounter = 0;
          if(nlcrCounter == 4) isHeader = false;
        } else {
          responseToReturn[responseLength++] = c;
          if(responseLength >= maxResponseLength) {
            wifiClient.stop();
            return -105;
          }
        }
      }
      rcvTimeout = millis() + (_OPENAI_TIMEOUT_DATA * 1000);
    } else {
      delay(100);
    }

    // Check if server has closed the connection
    if(!wifiClient.connected()) {
      rcvTimeout = 0;
      break;
    }
  }

  // Close client
  wifiClient.stop();

  // If no data has been received, or timeout value is still set (>0), it menas the opration timed out
  if((responseLength == 0) || (rcvTimeout != 0)) return -106;

  // If here, all was good. Return the length of the response body
  statusText(F("Data received"), 5);
  return responseLength;
}


/**
 * oledInitLayout
 * Init working layout on display
 */
void oledInitLayout() {
  display.clearDisplay();

  // App name
  oledPrint(APP_NAME, 4, 0, 1, 1, false);

  // Roll/pitch boxes
  display.drawRoundRect(0, 8, 128, 27, 2, WHITE);
  display.drawLine(64, 8, 64, 33, WHITE);
  oledPrint(F("Roll"), 3, 10, 1, 1, false);
  oledPrint(F("Pitch"), 67, 10, 1, 1, false);

  // Print photo size and transmission
  display.drawRoundRect(0, 36, 128, 19, 2, WHITE);
  display.drawLine(64, 36, 64, 52, WHITE);
  oledPrint(F("Photo size"), 3, 38, 1, 1, false);
  oledPrint(F("Transmit"), 67, 38, 1, 1, true);

  // Clear status bar
  statusText(F("System running..."), 1);
}


/**
 * oledPrint
 * Print text on oled
 * @param text    the text to display on the status bar
 * @param posX    text X position on display (-1 to print on the current cursor position)
 * @param posY    text Y position on display (-1 to print on the current cursor position)
 * @param sizeW   text size width
 * @param sizeH   text size height
 * @param flush   true to flush text on display
 */
void oledPrint(String text, int16_t posX, int16_t posY, uint8_t sizeW, uint8_t sizeH, bool flush) {
  uint8_t timeout = 0;
  uint16_t canary = random(1, 15000);

  // Wait canary for max 5 seconds
  while(__VISION.oledCanary != canary) {
    // Wait for canary to be released
    while(__VISION.oledCanary != 0) {
      timeout++;
      if(timeout > 200) return;
      delay(25);
    }

    // Try to get canary
    __VISION.oledCanary = canary;
  }

  // Print text
  if((posX >= 0) && (posY >= 0)) display.setCursor(posX, posY);
  display.setTextSize(sizeW, sizeH);
  display.print(text);
  if(flush) display.display();

  // Release canary
  __VISION.oledCanary = 0;
}


/**
 * statusText
 * Print text on status bar for a specified time
 * @param text      the text to display on the status bar
 * @param seconds   how many seconds to show the text
 */
void statusText(String text, uint8_t seconds) {
  char buf[22];
  sprintf(buf, "%-21s", text.substring(0, 21).c_str());
  oledPrint(buf, 0, 56, 1, 1, true);
  if(seconds != 0) __VISION.statusTextTimeout = millis() + (seconds * 1000);
}


/**
 * setRGB
 * Lights on the RGB led with specified color and for how long
 * @param red             red intensity, from 0 to RGB_BRIGHTNESS
 * @param green           green intensity, from 0 to RGB_BRIGHTNESS
 * @param blue            blue intensity, from 0 to RGB_BRIGHTNESS
 * @param milliseconds    how many milliseconds to keep the led turned on
 */
void setRGB(uint8_t red, uint8_t green, uint8_t blue, uint16_t milliseconds) {
  rgbLedWrite(RGB_BUILTIN, red, green, blue);
  __VISION.rgbLedTimeout = millis() + milliseconds;
}