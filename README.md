Smart Corridor Lighting System

An ESP32-S3-based intelligent corridor lighting system that combines PIR motion detection, ambient-light sensing, and time-based automation to reduce unnecessary electricity consumption.

Features

* Motion Detection: HC-SR501 PIR sensor detects movement in the monitored corridor.
* Ambient-Light Sensing: LDR module determines whether lighting is required.
* Time-Based Control: The system operates during the configured night-time window, from 6:00 PM to 5:00 AM.
* Automatic Lighting: The light switches on when motion is detected in darkness during active hours.
* Two-Minute Timeout: The light switches off after two minutes without further motion detection.
* Embedded Firmware: Developed in C using ESP-IDF and FreeRTOS.

Hardware Requirements

* ESP32-S3 development board
* HC-SR501 PIR motion sensor
* LDR module with digital output
* LED and current-limiting resistor for the prototype
* Jumper wires
* Suitable power supply

Software Requirements

* ESP-IDF
* Visual Studio Code
* ESP-IDF extension
* Git

System Logic

1. Read the LDR digital output.
2. Check whether the system is within its configured operating hours.
3. Detect movement using the PIR sensor.
4. Switch on the light when the system is active, the environment is dark, and motion is detected.
5. Keep the light on for two minutes after the most recent qualifying motion detection.
6. Switch the light off when the timeout expires or the operating conditions are no longer satisfied.

Pin Connections

Component	ESP32-S3 GPIO
LED control	GPIO 4
PIR output	GPIO 5
LDR digital output	GPIO 6

Confirm the pin assignments against your actual wiring before using the firmware.

Development Status

Prototype under development. The initial firmware uses a software clock for testing. A deployment-ready version should use a reliable real-time clock or network time synchronization.

Future Improvements

* Real-time clock synchronization
* Multiple corridor lighting zones
* Energy-consumption monitoring
* Web-based status dashboard
* Relay or MOSFET driver for controlling practical lighting loads

Author
Jayanth VD