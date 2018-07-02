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
    // Put IMU data acquisition code here

    // Save the output data in sensorData
    sensorData = "IMU data";
}

