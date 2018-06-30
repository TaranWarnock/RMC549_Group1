#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>

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
        GPSSensorThread() : SensorThread("GPS", "GPSdata") {}
};

class IMUSensorThread : public SensorThread {
    private:
        // Function for a single IMU reading
        void readFromSensor() override;

    public:
        IMUSensorThread() : SensorThread("IMU", "IMUdata") {}
};

#endif

