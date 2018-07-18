# Software Overview

## Raspberry Pi

The following directories and files contain code run by the Raspberry Pi:
* Command_and_Control
* Common
* Config
* I2C
* Logger
* Serial_Communication
* System_Control
* Telemetry
* main.py

## Arduino

The flight software for the arduino is found in the Arduino folder.
Arduino2 contains code for a secondary arduino which was planned as an isolated pressure sensor system on Rocky.
Arduino_backup contains an arduino program for use in case the Raspberry Pi failed, but was not needed for the launch.

## System Setup

To prepare the Arduino for flight, use the Arduino IDE to load `Arduino/arduino_thread_controller/arduino_thread_controller.ino` onto the Arduino.

To prepare the Raspberry Pi flight software, perform the following steps:
* Create a directory titled `RMC549Repos` in the Pi home directory
* Clone this repository into this new folder so that the path to the Flight Software Package looks like `/home/pi/RMC549Repos/RMC549_Group1/Flight_Software_Package`
* To enable automatic startup of the flight software, add the following four lines to the bottom of the file `/home/pi/.bashrc`:
```
sleep 20
python3 /home/pi/RMC549Repos/RMC549_Group1/ID_Broadcast/ID_broadcast.py &
sleep 20
python3 /home/pi/RMC549Repos/RMC549_Group1/Flight_Software_Package/main.py &
```
