/*
 * file: SensorThread.cpp
 *
 * Implementation of the SensorThread library with customized classes
 *   for each sensor connected to the arduino.
 */

#include "SensorThread.h"
#include <Wire.h>

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
    // address of IMU may also be 0x07
    uint8_t id =  IMUSensorThread::read8bit(0x28, 0x00);
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
        sensorData.concat(",");
    }
    else{
        sensorData.concat(",,,,,,,,,,,,,,,,,,,,,,,,,,");
    }


    if (Wire.requestFrom(0x60, 1, true) && pressureActive)
        IMUactive = true;
    else if (Wire.requestFrom(0x60, 1, true) && !pressureActive){
        // Reinitialise pressure sensor here
        preassurePtr->setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
        preassurePtr->setOversampleRate(7); // Set Oversample to the recommended 128
        preassurePtr->enableEventFlags(); // Enable all three pressure and temp event flags

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
        sensorData.concat(",");
    }
}


byte IMUSensorThread::read8bit(byte address, byte ID){
    byte value = 0;

    Wire.beginTransmission(address);    // begin I2C transmission with address
    #if ARDUINO >= 100                  // version of arduino IDE, ours will be > 100 as it is the newest version
        Wire.write((uint8_t)ID);        // queue bytes for transmission
    #else
        Wire.send(ID);
    #endif

    Wire.endTransmission();             // end transmission to slave device
    Wire.requestFrom(address, (byte)1); // request bytes from slave device

    #if ARDUINO >= 100
        value = Wire.read();            // read byte transmitted by slave device
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

// initialize geiger counter thread with the values of the interrupt pins each sensor is connected to
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
    // Save counts to sensor data
    // The main program in arduino_thread_controller.ino will use a timestamp to determine the 
    // duration of this sampling period.
    sensorData = "";
    sensorData.concat(String(m_eventCount[0]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[1]));
    sensorData.concat(",");
    sensorData.concat(String(m_eventCount[2]));

    // Reset counts
    m_eventCount[0] = 0;
    m_eventCount[1] = 0;
    m_eventCount[2] = 0;
}

// Interrupt Service Routine for a count event
void GeigerSensorThread::ISR1() {
    m_eventTime[0] = micros();
    if (m_eventTime[0] - m_eventTime[1] < 50)
    {
        // Simultaneous count occurred because the other geiger counter
        // detected a count within the last 50 microseconds
        m_eventCount[2]++;
        m_eventCount[1]--;
    }
    else
    {
        // Single count detected
        m_eventCount[0]++;
    }
}

void GeigerSensorThread::ISR2() {
    m_eventTime[1] = micros();
    if (m_eventTime[1] - m_eventTime[0] < 50)
    {
        // Simultaneous count detected
        m_eventCount[2]++;
        m_eventCount[0]--;
    }
    else
    {
        // Single count detected
        m_eventCount[1]++;
    }
}

// initialize the photosensor thread with handles to three different photosensors
PhotoSensorThread::PhotoSensorThread(TSL2561* tslPtr0, TSL2561* tslPtr1, TSL2561* tslPtr2) : SensorThread::SensorThread("PHOTO", "VIS0,IR0,VIS1,IR1,VIS2,IR2") {
    m_tslPtr[0] = tslPtr0;
    m_tslPtr[1] = tslPtr1;
    m_tslPtr[2] = tslPtr2;
}

void PhotoSensorThread::readFromSensor() {
    // Read in the values from 3 photosensors connected to the arduino.
    // The method getFullLuminosity() returns a 32 bit value where the
    // top 16 bits provide the IR luminosity and the bottom 16 bits
    // provide the full (visible + infrared) spectrum. Follows the
    // example at https://github.com/adafruit/TSL2561-Arduino-Library/blob/master/examples/tsl2561/tsl2561.ino

    uint32_t lum0, lum1, lum2;
    uint16_t ir0, ir1, ir2, full0, full1, full2;

    lum0 = m_tslPtr[0]->getFullLuminosity();
    ir0 = lum0 >> 16;       // shift by 16 bits to get IR
    full0 = lum0 & 0xFFFF;  // do AND operation with bit mask "0000 0000 0000 0000 1111 1111 1111 1111" to get full spectrum
    // to get the visible portion we would do vis0 = full0 - ir0

    lum1 = m_tslPtr[1]->getFullLuminosity();
    ir1 = lum1 >> 16;
    full1 = lum1 & 0xFFFF;

    lum2 = m_tslPtr[2]->getFullLuminosity();
    ir2 = lum2 >> 16;
    full2 = lum2 & 0xFFFF;

    // create data string
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

