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
//        String line1 = "";
//        String line2 = "";
//        String temp = "";


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
                    sensorData.concat(c);
            }

        }

//        while(Serial1.available()){
//            char c = Serial1.read();

//            if (c == '$'){
//                startOfNewNMEA = true;
//                line2 = line1;
//                line1 = temp;
//                temp = "";
//            }

//            if (startOfNewNMEA)
//                temp.concat(c);

//        }

//            sensorData.concat(line1);
//            sensorData.concat(line2);
            sensorData.trim();
            sensorData.replace("\n",",");
            sensorData.replace("\r",",");

        // UPD (LO): This code will provide two NMEA lines which contain the __one and only required__ information.
        // It might be left as it is now and processed later not to take operational time from the main loop.
        // Parsing the comma separated string shouldn't be a problem

        // UPD (LO): We want to make GPS module constantly running and spit out data on request. I have to talk to Lukas and Tom about it because of
        // the analogy to geiger counter.



}

void IMUSensorThread::readFromSensor() {
    // Put IMU data acquisition code here

    // Save the output data in sensorData
    sensorData = "IMU data";
}

