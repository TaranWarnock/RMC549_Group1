#include "SensorThread.h"

void SensorThread::run() {
    readFromSensor();

    runned();
}

// Implement the code for a single GPS reading event here
void GPSSensorThread::readFromSensor() {
    // Put the GPS aquisition code here

    // Save the output data in sensorData
    sensorData = "GPS data";
}

void IMUSensorThread::readFromSensor() {
    // Put IMU data aquisition code here
    
    // Save the output data in sensorData
    sensorData = "IMU data";
}

