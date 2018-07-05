#include "SensorThread.h"

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

        while(Serial1.available())
          Serial1.read();
        while(Serial1.available())
          Serial1.read();
//        while(Serial1.available())
//          Serial1.read();
//        while(Serial1.available())
//          Serial1.read();


        while(true)
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
            sensorData = "NaN,NaN,NaN,NaN,NaN";
            return;
        }
        if (!NMEA2[NMEA2.length()-4] == '*') {
            sensorData = "NaN,NaN,NaN,NaN,NaN";
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
            if (valueCounter == 2 || valueCounter == 3 || valueCounter == 5 || valueCounter == 8 || valueCounter == 10 || valueCounter == 11)
                sensorData.concat(GPGGA.substring(oldIndx + 1, newIndx + 1));

            oldIndx = newIndx;
            newIndx = GPGGA.indexOf(",", oldIndx + 1);
        }
        
        sensorData.remove(sensorData.length() - 1);

}

void IMUSensorThread::readFromSensor() {
    // Put IMU data acquisition code here and save result in sensorData
    sensorData = "";
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
    sensorData.concat(preassurePtr->readPressure());
    sensorData.concat(",");
    sensorData.concat(preassurePtr->readTemp());
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

PhotoSensorThread::PhotoSensorThread(TSL2561* tslPtr) : SensorThread::SensorThread("PHOTO", "VIS,IR") {
    m_tslPtr = tslPtr; 
}

void PhotoSensorThread::readFromSensor() {
    
    uint32_t lum = m_tslPtr->getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    
    sensorData = "";
    sensorData.concat(String(full));
    sensorData.concat(",");
    sensorData.concat(String(ir));
}

