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

GeigerSensorThread::GeigerSensorThread(int interruptPin1, int interruptPin2) : SensorThread::SensorThread("GEIGER") {
    m_interruptPin[0] = interruptPin1;
    m_interruptPin[1] = interruptPin2;
    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventTime[0] = 0;
    m_eventTime[1] = 0; 

    pinMode(m_interruptPin[0], INPUT);
    pinMode(m_interruptPin[1], INPUT);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[0]), &GeigerSensorThread::ISR1, FALLING);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[1]), &GeigerSensorThread::ISR2, FALLING);
}

void GeigerSensorThread::readFromSensor() {
    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventCount[2] = 0;

    // send stuff to Pi (don't know how ATM)
}

void GeigerSensorThread::ISR1() {
    m_eventTime[0] = micros();
    if (m_eventTime[0] - m_eventTime[1] < 50)
    {
        m_eventCount[2]++;
        m_eventCount[1]--;
    }
    else
    {
        m_eventCount[0]++;
    }
}

void GeigerSensorThread::ISR2() {
    m_eventTime[1] = micros();
    if (m_eventTime[1] - m_eventTime[0] < 50)
    {
        m_eventCount[2]++;
        m_eventCount[0]--;
    }
    else
    {
        m_eventCount[1]++;
    }
}
