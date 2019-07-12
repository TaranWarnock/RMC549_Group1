/*
 * file: SensorThread.cpp
 *
 * Implementation of the SensorThread library with customized classes
 *   for each sensor connected to the arduino.
 */

#include "SensorThread.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// Generic function for sampling from a sensor
void SensorThread::run() {
    // Call the sensor specific function to save data in sensorData
    readFromSensor();

    // The Arduino Thread library requires this line at the end of run()
    runned();
}


void GPSSensorThread::readFromSensor() {
        bool startOfNewNMEA = false;
        bool myInterrupt = false;
        int lineCount = 0;
        sensorData = "";
        String NMEA1 = "";
        String NMEA2 = "";
        String GPGGA = "";
        String GPVTG = "";
//        int timeCheck = 8000;
//        active = true;

        while(Serial1.available())
          Serial1.read();
        while(Serial1.available())
          Serial1.read();
//        while(Serial1.available())
//          Serial1.read();
//        while(Serial1.available())
//          Serial1.read();

        float startingTime = millis();

        while(true && millis() < startingTime + timeCheck)
        {
            if (Serial1.available() > 0){
                char c = Serial1.read();
                if (c == '$'){
                    startOfNewNMEA = true;
                    lineCount++;
                    if (lineCount == 2){
                        NMEA1 = NMEA2;
                        NMEA2 = "";
                    }
                }
                if (lineCount > 2){
                    myInterrupt = true;
                    break;
                }
                if (startOfNewNMEA)
                    NMEA2.concat(c);
            }
        }

        if (millis() >= startingTime + timeCheck){
//            active = false;
            sensorData = ",,,,,,,";
            timeCheck = 2000;
            return;
        }

        timeCheck = 3000;
        NMEA1.trim();
        NMEA1.replace("\n","");
        NMEA1.replace("\r","");
        NMEA2.trim();
        NMEA2.replace("\n","");
        NMEA2.replace("\r","");
//        sensorData.concat("|");
//        sensorData.concat(NMEA1);
//        sensorData.concat("|");
//        sensorData.concat(NMEA2);

//        Sentances were not read or recieved properly
        if (!NMEA1[NMEA1.length()-4] == '*') {
            sensorData = ",,,,,,,";
            return;
        }
        if (!NMEA2[NMEA2.length()-4] == '*') {
            sensorData = ",,,,,,,";
            return;
        }

        if (NMEA1.startsWith("$GPGGA")){
            GPGGA = NMEA1;
            GPVTG = NMEA2;
        }
        else {
            GPVTG = NMEA1;
            GPGGA = NMEA2;
        }

//        sensorData.concat(NMEA);
//        sensorData.concat("|");

//        Processing GPGGA sentance
        int valueCounter = 0;
        int oldIndx = 0;
        int newIndx = GPGGA.indexOf(",");

        while(newIndx > 0){
            valueCounter++;
            if (valueCounter == 2 || valueCounter == 3 || valueCounter == 4 || valueCounter == 5 || valueCounter == 6 || valueCounter == 8 || valueCounter == 10 || valueCounter == 11)
                sensorData.concat(GPGGA.substring(oldIndx + 1, newIndx + 1));

            oldIndx = newIndx;
            newIndx = GPGGA.indexOf(",", oldIndx + 1);
        }

        sensorData.remove(sensorData.length() - 1);

}

void IMUSensorThread::readFromSensor() {
    sensorData = "";

    if (Wire.requestFrom(0x28, 1, true) && IMUactive)
        IMUactive = true;
    else if (Wire.requestFrom(0x28, 1, true) && !IMUactive){
        // Detected IMU after a disconnection or power failure
        // Reinitialise IMU here
        *bnoPtr = Adafruit_BNO055(55);
        IMUactive = bnoPtr->begin();
    }
    else if (!Wire.requestFrom(0x28, 1, true))
        // Detected that IMU is not connected
        IMUactive = false;

    // Put IMU data acquisition code here and save result in sensorData
    if (IMUactive){
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_ACCELEROMETER, "A"));
        sensorData.concat(",");
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_GYROSCOPE, "Gy"));
        sensorData.concat(",");
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_MAGNETOMETER , "M"));
        sensorData.concat(",");
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_EULER, "E"));
        sensorData.concat(",");
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_LINEARACCEL, "L"));
        sensorData.concat(",");
        sensorData.concat(getvec(Adafruit_BNO055::VECTOR_GRAVITY, "Gr"));
        sensorData.concat(",");

        // get temperature and append to sensorData (accuracy of sensor is 1 degree)
        int temp = bnoPtr->getTemp();
        sensorData.concat(String(temp));
        sensorData.concat(",");
        sensorData.concat(displayCalStatus());
    }
    else{
        sensorData.concat(",,,,,,,,,,,,,,,,,,,,,,,,,,");
    }
}

String IMUSensorThread::getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type,
                               String title) {
    imu::Vector<3> data_vector = bnoPtr->getVector(sensor_type);
    String vecString = "";
    vecString = String(data_vector[0]) + "," + String(data_vector[1]) + "," + String(data_vector[2]);

    return vecString;
}

String IMUSensorThread::displayCalStatus(void) {
    /* Get the four calibration values (0..3) */
    /* Any sensor data reporting 0 should be ignored, */
    /* 3 means 'fully calibrated" */
    uint8_t system, gyro, accel, mag;
    system = gyro = accel = mag = 0;
    String calString = "";
    bnoPtr->getCalibration(&system, &gyro, &accel, &mag);
    /* The data should be ignored until the system calibration is > 0 */
    //calString.concat(",");
    //if (!system) {
    //  calString.concat("! ");
    //}
    /* Display the individual values */
    calString = String(system) + "," + String(gyro) + "," + String(accel) + "," + String(mag);

    return calString;
}

// define static member variables for Geiger class
volatile int GeigerSensorThread::m_interruptPin[] = {0, 0, 0, 0};
volatile uint16_t GeigerSensorThread::m_eventCount[] = {0, 0, 0};
volatile unsigned int GeigerSensorThread::m_eventTime[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0,  };
volatile int GeigerSensorThread::m_timearrayctr[] = {0};

// initialize geiger counter thread with the values of the interrupt pins each sensor is connected to
GeigerSensorThread::GeigerSensorThread(int interruptPin1, int interruptPin2, int interruptPin3, int interruptPin4) : SensorThread::SensorThread("GEIGER", "C1,C2,GN") {
    m_interruptPin[0] = interruptPin1;
    m_interruptPin[1] = interruptPin2;
    m_interruptPin[2] = interruptPin3;
    m_interruptPin[3] = interruptPin4;
    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventCount[2] = 0;
    for (int i = 0; i < 101; i++) {
        m_eventTime[i] = 0;
        
    }
    m_timearrayctr[0] = 0;

    pinMode(m_interruptPin[0], INPUT_PULLUP);
    pinMode(m_interruptPin[1], INPUT_PULLUP);
    pinMode(m_interruptPin[2], INPUT_PULLUP);
    pinMode(m_interruptPin[3], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[0]), &GeigerSensorThread::ISR1, FALLING);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[1]), &GeigerSensorThread::ISR2, FALLING);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[2]), &GeigerSensorThread::ISR3, FALLING);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[3]), &GeigerSensorThread::ISR4, FALLING);
}

void GeigerSensorThread::readFromSensor() {
    // Save counts to sensor data
    // The main program in arduino_thread_controller.ino will use a timestamp to determine the 
    // duration of this sampling period.
    sensorData = "";
    sensorData.concat(String(m_eventCount[0]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[1]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[2]));

    Serial.begin(9600);
    if (!SD.begin(4)) {
    // Serial.println("Card failed, or not present");
    // don't do anything more:
    // while (1);
  }

    File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    for (int j = 0; j < 99; j++) {
        dataFile.print(m_eventTime[j]);
        dataFile.print(" ");
        m_eventTime[j] = 0;
        
    }
    dataFile.print('\n');
    
    dataFile.close();
    // print to the serial port too:
    // Serial.println(dataString);
  }

  m_timearrayctr[0] = 0;
    // Reset counts
    //m_eventCount[0] = 0;
    //m_eventCount[1] = 0;
    //m_eventCount[2] = 0;
}

// Interrupt Service Routine for a count event for Geiger Counter 1
void GeigerSensorThread::ISR1() {
    m_eventTime[m_timearrayctr[0]] = millis();
    m_eventCount[0]++; // add a count
    m_timearrayctr[0]++;
    }

// Interrupt Service Routine for a noise event for Geiger Counter 1
void GeigerSensorThread::ISR2() {
    m_eventCount[0]--; // Remove a count as this count was likely noise
    m_eventCount[2]++;
    m_timearrayctr[0]--;
}

// Interrupt Service Routine for a count event for Geiger Counter 2
void GeigerSensorThread::ISR3() {
    m_eventTime[m_timearrayctr[0]] = millis();
    m_eventCount[1]++;
    m_timearrayctr[0]++;
    }

// Interrupt Service Routine for a noise event for Geiger Counter 2
void GeigerSensorThread::ISR4() {
    m_eventCount[1]--;
    m_eventCount[2]++;
    m_timearrayctr[0]--;
}