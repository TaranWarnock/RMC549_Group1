# Software Overview

## serial_communication.py

This file is used to communicate with the Arduino (recieving downlink data and sending uplink data) and logs the data to a .txt file on computer. See documentation within the file for how to operate.

## live_plotting.py

This file is used to collected and parse the data from the log .txt files and plot them in real time during the flight for the ground station operators

## generate_dummy_logs.py

This file is used to generate dummy log files for the purpose of running/debuging the live_plotting.py script without actually being connected to a LoRa device and recieving data from another LoRa device. See documentation within the file for how to operate.

## arduino_ground Folder

This folder contains the file of the program that runs on the ground station arduino.