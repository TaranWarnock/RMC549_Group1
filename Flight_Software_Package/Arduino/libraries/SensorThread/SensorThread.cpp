#include "SensorThread.h"

void SensorThread::run() {
    readFromSensor();

    runned();
}

// Implement the code for a single GPS reading event here
void GPSSensorThread::readFromSensor() {
    // read from sensor and store in buffer
    Serial.println("GPS Sensor Class");
    sensorData = "GPD, sensor data here";
}
