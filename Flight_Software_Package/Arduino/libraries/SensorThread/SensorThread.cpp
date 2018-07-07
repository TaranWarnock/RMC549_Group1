#include "SensorThread.h"
#include <Wire.h>

void SensorThread::run() {
    readFromSensor();

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
            sensorData = ",,,,,";
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
            sensorData = ",,,,,";
            return;
        }
        if (!NMEA2[NMEA2.length()-4] == '*') {
            sensorData = ",,,,,";
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
    // address of IMU may also be 0x07
    uint8_t id =  IMUSensorThread::read8bit(0x28, 0x00);
    sensorData = "";

    if (Wire.requestFrom(0x28, 1, true) && IMUactive)
        IMUactive = true;
    else if (Wire.requestFrom(0x28, 1, true) && !IMUactive){
        // Reinitialise IMU here
        IMUactive = true;
    }
    else if (!Wire.requestFrom(0x28, 1, true))
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
        sensorData.concat(",");
    }
    else{
        sensorData.concat("!NO_IMU!_");
    }


    if (Wire.requestFrom(0x60, 1, true) && pressureActive)
        IMUactive = true;
    else if (Wire.requestFrom(0x60, 1, true) && !pressureActive){
        // Reinitialise IMU here
        pressureActive = true;
    }
    else if (!Wire.requestFrom(0x60, 1, true))
        pressureActive = false;


    if (pressureActive){
        sensorData.concat(preassurePtr->readPressure());
        sensorData.concat(",");
        sensorData.concat(preassurePtr->readTemp());
    }
    else {
        sensorData.concat("!NO_PRESSURE!_");
    }
}


byte IMUSensorThread::read8bit(byte address, byte ID){
    byte value = 0;

    Wire.beginTransmission(address);
    #if ARDUINO >= 100
        Wire.write((uint8_t)ID);
    #else
        Wire.send(ID);
    #endif

    Wire.endTransmission();
    Wire.requestFrom(address, (byte)1);

    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif

    return value;
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
volatile int GeigerSensorThread::m_interruptPin[] = {0, 0};
volatile uint16_t GeigerSensorThread::m_eventCount[] = {0, 0, 0};
volatile unsigned long GeigerSensorThread::m_eventTime[] = {0, 0};

GeigerSensorThread::GeigerSensorThread(int interruptPin1, int interruptPin2) : SensorThread::SensorThread("GEIGER", "C1,C2,SC") {
    m_interruptPin[0] = interruptPin1;
    m_interruptPin[1] = interruptPin2;
    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventCount[2] = 0;
    m_eventTime[0] = 0;
    m_eventTime[1] = 0; 

    pinMode(m_interruptPin[0], INPUT);
    pinMode(m_interruptPin[1], INPUT);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[0]), &GeigerSensorThread::ISR1, FALLING);
    attachInterrupt(digitalPinToInterrupt(m_interruptPin[1]), &GeigerSensorThread::ISR2, FALLING);
}

void GeigerSensorThread::readFromSensor() {
    // save counts to sensor data
    sensorData = "";
    sensorData.concat(String(m_eventCount[0]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[1]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[2]));

    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventCount[2] = 0;
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

PhotoSensorThread::PhotoSensorThread(TSL2561* tslPtr0, TSL2561* tslPtr1, TSL2561* tslPtr2) : SensorThread::SensorThread("PHOTO", "VIS0,IR0,VIS1,IR1,VIS2,IR2") {
    m_tslPtr[0] = tslPtr0;
    m_tslPtr[1] = tslPtr1;
    m_tslPtr[2] = tslPtr2;
}

void PhotoSensorThread::readFromSensor() {
    
    uint32_t lum0, lum1, lum2;
    uint16_t ir0, ir1, ir2, full0, full1, full2;

    lum0 = m_tslPtr[0]->getFullLuminosity();
    ir0 = lum0 >> 16;
    full0 = lum0 & 0xFFFF;
    
    lum1 = m_tslPtr[1]->getFullLuminosity();
    ir1 = lum1 >> 16;
    full1 = lum1 & 0xFFFF;
    
    lum2 = m_tslPtr[2]->getFullLuminosity();
    ir2 = lum2 >> 16;
    full2 = lum2 & 0xFFFF;
    
    sensorData = "";
    sensorData.concat(String(full0));
    sensorData.concat(",");
    sensorData.concat(String(ir0));
    sensorData.concat(",");
    sensorData.concat(String(full1));
    sensorData.concat(",");
    sensorData.concat(String(ir1));    
    sensorData.concat(",");
    sensorData.concat(String(full2));
    sensorData.concat(",");
    sensorData.concat(String(ir2));
}

