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
    sensorData.concat(getvec(Adafruit_BNO055::VECTOR_GYROSCOPE, "Gy"));
    sensorData.concat(getvec(Adafruit_BNO055::VECTOR_MAGNETOMETER , "M"));
    sensorData.concat(getvec(Adafruit_BNO055::VECTOR_EULER, "E"));
    sensorData.concat(getvec(Adafruit_BNO055::VECTOR_LINEARACCEL, "L"));
    sensorData.concat(getvec(Adafruit_BNO055::VECTOR_GRAVITY, "Gr"));

    // get temperature and append to sensorData (accuracy of sensor is 1 degree)
    int temp = bnoPtr->getTemp();
    sensorData.concat("T: ");
    sensorData.concat(String(temp));
    sensorData.concat(" C");
    sensorData.concat(displayCalStatus());
}

String IMUSensorThread::getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type,
                               String title) {
    imu::Vector<3> data_vector = bnoPtr->getVector(sensor_type);
    String vecString = "";
    vecString.concat(title);
    vecString.concat(": X: ");
    vecString.concat(String(data_vector[0]));
    vecString.concat(" Y: ");
    vecString.concat(String(data_vector[1]));
    vecString.concat(" Z: ");
    vecString.concat(String(data_vector[2]));

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
    calString.concat(",");
    if (!system) {
      calString.concat("! ");
    }
    /* Display the individual values */
    calString.concat("Calibration: Sys:");
    calString.concat(String(system));
    calString.concat(" G:");
    calString.concat(String(gyro));
    calString.concat(" A:");
    calString.concat(String(accel));
    calString.concat(" M:");
    calString.concat(String(mag));

    return calString;
}

