#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>   // used in IMU code

class SensorThread : public Thread {
    protected:
        String sensorName;
		String sensorHeader;
        String sensorData;

        // Function for a single sensor reading event here
        virtual void readFromSensor()  = 0;

    public:
        // constructor should set sensor name and establish data header (names of data vars in order)
        SensorThread() : Thread() {}
        SensorThread(String name, String header) : Thread() {
            sensorName   = name;
			sensorHeader = header;
        }
        
        void run() override;
		
		String getSensorHeader() {
            return sensorHeader;
        }
		
        String getSensorData() { 
            return sensorData;
        }

        String getSensorName() {
            return sensorName;
        }
};

class EmuSensorThread : public SensorThread {
    private:
        // Function for emulation of a generic sensor
        void readFromSensor() override;

    public:
        EmuSensorThread() : SensorThread("EMU", "RndNumOne,RndNumTwo") {}
};

class GPSSensorThread : public SensorThread {
    private:
        // Function for a single GPS reading
        void readFromSensor() override;

    public:
        GPSSensorThread() : SensorThread("GPS", "NMEA1,NMEA2") {}
};

class IMUSensorThread : public SensorThread {
    private:
        Adafruit_BNO055* bnoPtr;    // pointer to sensor handler

        // Function for a single IMU reading
        void readFromSensor() override;
		
		String getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type, String title);
		String displayCalStatus(void);

    public:
        IMUSensorThread(Adafruit_BNO055* bno) : SensorThread("IMU", "IMUdata") {
            bnoPtr = bno;
        }
};

#endif

