# Wiz Horizon Observer

Wiz Horizon Observer (WHO) is a PoC (Proof of Concept) project to evaluate the feasibility of an automatic optical system to detect approaching boats or ships on the horizon from a rescue and salvage ship.

The PoC has been build using an ESP32-S3-WROOM-1-N16R8 microcontroller board that support WiFi networking and mounts a 5MP builtin camera.
The hardware structure has been build with Lego Technics pieces.

The idea is to have a stabilized camera that points towards the horizon, and regularly takes pictures that are then analyzed with AI trying to identify approaching boats or ships.

The stabilized camera is based on an MPU-6050 Accelerometer and Gyroscope Sensor that's used to track the rescue ship's roll and pitch due to the sea movements, to keep an as stable as possible view of the horizon.
The ESP32 board constantly monitors the roll and pitch and drives two servo motors mounted on the roll and pitch axis to compensate the movements.

In this PoC project the horizon pictures are taken pressing the Boot/IO0 button on the ESP32 board. In a production environment, pictures are taken automatically every minute.
The picture is then sent to an AI system (tests have been done with OpenAI/ChatGPT) asking the probability that there's a boat or a ship on the horizon.
The result is a value from 1 (no boats) to 4 (boat detected) that described the probability of a boat identified on the horizon. Values of 2 and 3 means a probability of 33% and 66%.

Tests can be executed without an OpenAI (paid) account, via a test server that simulates the response received from OpenAI. The test server is a PHP script that logs the request, save the picture and sends back a valid response with a random (1 to 4) probability.
The test script is available in the testWhoServer directory. To use the test server please change the parameters in the config.h file to point to your hosting server.

An OLED display shows info about the system.
When booting, it shows the status of all the sensors when being activated, and also info about the connection to the WiFi network.
When running, it constantly shows the roll and pitch degrees, and info about the picture being taken, transmitted and analyzed.
When the picture analysis is complete, a message is shown on the display with the analysis result.
The RGB led also shows a different colour depending on the analysis result: 1 (0% probability) is green, 2 (33%) is orange, 3 (66%) is violet, 4 (100%) is red.

ESP32 and other devices conenctions are in the PCB directory.
Some pictures of the stabilized camera structure are also in the PCB directory.


# Required Libraries
- base64_encode by dojyorin
- ArduinoJson by Benoit Blanchon
- ESP32Servo by Kevin Harrington and John K. Bennett
- Adafruit SSD1306 by Adafruit


## Credits and references
Created by [WizLab.it](https://www.wizlab.it/)

GitHub: [wizlab-it/wiz-horizon-observer](https://github.com/wizlab-it/wiz-horizon-observer/)


## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
To get a copy of the GNU General Public License, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/)