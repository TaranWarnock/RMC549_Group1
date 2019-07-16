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
#include <Adafruit_Sensor.h>
#include <Sydafruit_TSL2561_U.h>

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

int LightSensorThread::Start3Light() {
  // Tell the sensors to begin sensing
  now = millis();
  lux1.startCount(now);
  lux2.startCount(now);
  lux3.startCount(now);
}

void LightSensorThread::readFromSensor() {

    sensorData = ""; // Needed to clear data from last measurement

    tsl2561IntegrationTime_t timeSetting = TSL2561_INTEGRATIONTIME_13MS; //actually a bitwise command

    // variables that check connection errors
    int i = 0;
    int j = 0;
    int k = 0;
    bool light_sensor_1_error = false;
    bool light_sensor_2_error = false;
    bool light_sensor_3_error = false;

    // Don't need to run through this every time, just the first time
    // Ideally it would be in the creator class but it will freeze the code if placed there
    //(as it will not be able to find the sensors)
    if (activated == false) { 
     while(true){
        if (!lux1.begin()) {  // Try to initialise the sensor
            i++; // if it fails, add a count
            if (i>5){ // if it fails more than five times, there is a connection error. Log it and break.
                light_sensor_1_error = true;
                break;
            }
            continue; // Not ready yet, try again
      } else {
            lux1.setGainTime(TSL2561_GAIN_1X, timeSetting); 
            break; // Initialization complete, break out to start next
      }
     }

    while(true){
        if (!lux2.begin()) {  // Try to initialise the sensor
            j++;
            if (j>5){
                light_sensor_2_error = true;
                break;
            }
            continue; // Not ready yet, try again
      } else {
            lux2.setGainTime(TSL2561_GAIN_1X, timeSetting); 
            break; // Initializion complete, break out to start next
      }
  }

    while(true){
        if (!lux3.begin()) {  // Try to initialise the sensor
            k++;
            if (k>5){
                light_sensor_3_error = true;
                break;
            }
            continue; // Not ready yet, try again
      } else {
            lux3.setGainTime(TSL2561_GAIN_1X, timeSetting); 
            break; // Initialization complete, ready to continue!
      }
  }

  activated = true; // This has run once, tell it not to run again
}

    Start3Light(); // Start sensors for integration
    while(true){
        now = millis();
        if (now - lux1._tsl2561Counting < lightIntegrate) {
            continue; // Wait until the integration time has passed
        } 
        else { // Integration time has passed, grab data and send
            noInterrupts(); //I don't know what happens if you interrupt a bitwise command to the cricuitry, let's not find out
            lux1.readADC(&BBLight1, &IRLight1);
            lux2.readADC(&BBLight2, &IRLight2);
            lux3.readADC(&BBLight3, &IRLight3);
            interrupts();
            if ((BBLight1 == 65535) && (IRLight1 == 65535)){
                light_sensor_1_error = true;
            }
            if ((BBLight2 == 65535) && (IRLight2 == 65535)){
                light_sensor_2_error = true;
            }
            if ((BBLight3 == 65535) && (IRLight3 == 65535)){
                light_sensor_3_error = true;
            }

            if (light_sensor_1_error == false) {
                sensorData.concat(String(BBLight1));
                sensorData.concat(",");
                sensorData.concat(String(IRLight1));
                sensorData.concat(",");
            }
            else {
                sensorData.concat("LIGHT_SENSOR_1_ERROR,LIGHT_SENSOR_1_ERROR,");
            }
            if (light_sensor_2_error == false) {
                sensorData.concat(String(BBLight2));
                sensorData.concat(",");
                sensorData.concat(String(IRLight2));
                sensorData.concat(",");
            }
            else {
                sensorData.concat("LIGHT_SENSOR_2_ERROR,LIGHT_SENSOR_2_ERROR,");
            }
            if (light_sensor_3_error == false) {
                sensorData.concat(String(BBLight3));
                sensorData.concat(",");
                sensorData.concat(String(IRLight3));
                sensorData.concat(",");
            }
            else {
                sensorData.concat("LIGHT_SENSOR_3_ERROR,LIGHT_SENSOR_3_ERROR,");
            }
            break;
        }
    }
}

LightSensorThread::LightSensorThread():SensorThread::SensorThread("LIGHT", "BBL1,IRL1,BBL2,IRL2,BBL3,IRL3") {

    // Prepare to connect to light sensors
    pinMode(13, OUTPUT);
    Serial.begin(9600);

    // These must be instantiated both here and in the header file, or the compiler cannot find the correct library function
    // Don't ask me why
     Adafruit_TSL2561_Unified lux1 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);
     Adafruit_TSL2561_Unified lux2 = Adafruit_TSL2561_Unified(TSL2561_ADDR_HIGH, 12346);
     Adafruit_TSL2561_Unified lux3 = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12347);

     // If necessary, change the integration time here
     lightIntegrate = TSL2561_DELAY_INTTIME_13MS;
     
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

    // Reset counts
    m_eventCount[0] = 0; // Geiger counter 1
    m_eventCount[1] = 0; // Geiger counter 2
    m_eventCount[2] = 0; // Noise counts from both counters combined

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

}

// Interrupt Service Routine for a count event for Geiger Counter 1
void GeigerSensorThread::ISR1() {
    m_eventTime[m_timearrayctr[0]] = millis();
    m_eventCount[0]++; // add a count
    if (m_timearrayctr[0] < 99){ // So as not to crash the program if there is a sudden burst of radiation and the array overflows
            m_timearrayctr[0]++;
        }
    }

// Interrupt Service Routine for a noise event for Geiger Counter 1
void GeigerSensorThread::ISR2() {
    m_eventCount[0]--; // Remove a count as this count was likely noise
    m_eventCount[2]++;
    if (m_timearrayctr[0] > 1){ // So as not to crash the program if a ton of noise happens and it tries to go below 0
        m_timearrayctr[0]--;
    }
}

// Interrupt Service Routine for a count event for Geiger Counter 2
void GeigerSensorThread::ISR3() {
    m_eventTime[m_timearrayctr[0]] = millis();
    m_eventCount[1]++;
    if (m_timearrayctr[0] < 99){
        m_timearrayctr[0]++;
        }
    }

// Interrupt Service Routine for a noise event for Geiger Counter 2
void GeigerSensorThread::ISR4() {
    m_eventCount[1]--;
    m_eventCount[2]++;
    if (m_timearrayctr[0] > 1){
        m_timearrayctr[0]--;
    }
}