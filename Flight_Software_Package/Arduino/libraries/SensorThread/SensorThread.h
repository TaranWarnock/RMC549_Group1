#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>

class SensorThread : public Thread {
    protected:
        String sensorName;
        String sensorData;

        // Function for a single sensor reading event here
        virtual void readFromSensor()  = 0;

    public:
        // constructor should set sensor name
        SensorThread() : Thread() {}
        SensorThread(String name) : Thread() {
            sensorName = name;
        }
        
        void run() override;

        String getSensorData() { 
            return sensorData;
        }
};

class GPSSensorThread : public SensorThread {
    private:
        // Function for a single GPS reading
        void readFromSensor() override;

    public:
        GPSSensorThread() : SensorThread("GPS") {}
};

class IMUSensorThread : public SensorThread {
    private:
        // Function for a single IMU reading
        void readFromSensor() override;

    public:
        IMUSensorThread() : SensorThread("IMU") {}
};

#endif

