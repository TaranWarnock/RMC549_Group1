#include "SensorThread.h"

void SensorThread::run() {
    readFromSensor();

    runned();
}

void EmuSensorThread::readFromSensor() {
    // Just generate random numbers for emulation
	long random_data_one = random(0, 1000.01);
	long random_data_two = random(0, 1000.01);

    // Save the output data in sensorData
    sensorData = String(random_data_one);
	sensorData.concat(",");
	sensorData.concat(random_data_two);
}


void GPSSensorThread::readFromSensor() {
        bool startOfNewNMEA = false;
        bool myInterrupt = false;
        int lineCount = 0;
        sensorData = "";
        String NMEA = "";


        while(Serial1.available())
          Serial1.read();
        while(Serial1.available())
          Serial1.read();
        while(Serial1.available())
          Serial1.read();
        while(Serial1.available())
          Serial1.read();


        while(true)
        {
            if (Serial1.available() > 0){
                char c = Serial1.read();
                if (c == '$'){
                    startOfNewNMEA = true;
                    lineCount++;
                }
                if (lineCount > 2){
                    myInterrupt = true;
                    break;
                }
                if (startOfNewNMEA)
                    NMEA.concat(c);
            }
        }

        NMEA.trim();
        NMEA.replace("\n",",");
        NMEA.replace("\r",",");

        if (!NMEA[NMEA.length()-4] == '*') {
            sensorData = "NaN";
            return;
        }


//        Sorting, so GPGGA will be before GPVTG
        if (NMEA.startsWith("$GPVTG")){
            int indx = NMEA.indexOf("$GPGGA");
            NMEA = NMEA.substring(indx) + "," + NMEA.substring(0, indx);
        }

//        sensorData.concat(NMEA);
//        sensorData.concat("|");

        int valueCounter = 0;
        int oldIndx = 0;
        int newIndx = NMEA.indexOf(",");

        while(newIndx > 0){
            valueCounter++;
            if (valueCounter == 2 || valueCounter == 3 || valueCounter == 5 || valueCounter == 10 || valueCounter == 24){
                sensorData.concat(NMEA.substring(oldIndx + 1, newIndx + 1));
            }
            oldIndx = newIndx;
            newIndx = NMEA.indexOf(",", oldIndx + 1);
        }


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

GeigerSensorThread::GeigerSensorThread(int interruptPin1, int interruptPin2) : SensorThread::SensorThread("GEIGER", "Count1,Count2,SimultaneousCount") {
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
    // send stuff to Pi (don't know how ATM)
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
