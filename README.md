# RMC 549 Software Repository
This repository contains the code used by both group 0 (Rocky) and group 1 (Major Tom).
Given that Major Tom was the payload which flew, the master branch is up to date for this payload.
To view Rocky's version of the flight software, view the "Rocky" branch which includes the light sensor implementation.

## Directories
* Code_Examples - Contains an example of how to use the Arduino Thread library
* Flight_Software_Package - Contains the software suite for payload data collection, storage, and transmission using a Raspberry Pi and Arduino
* Ground_Software_Package - Contains the software used by the telemetry ground station
* ID_Broadcast - Contains python scripts enabling the payload to transmit its IP and data and for a computer to receive this information
* Link_Budget - Contains python scripts to calculate the APRS and LoRa link budgets
* Schematics - Contains fritzing PCB schematics
* talking_to_sensors - Contains development code used in testing and setting up various sensors. This code was later finalized and integrated into the flight software package
