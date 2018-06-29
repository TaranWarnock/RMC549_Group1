#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>

class SensorThread : public Thread {
    private:
        char* sensorName;
        char* sensorData;

        // the specific sensor behaviour will be implemented by this function
        virtual void callback() {}

    public:
        // constructor should set sensor name and size of data array
        SensorThread() : Thread() {}
        
        void run() override;
};

class GPSSensorThread : public SensorThread {
    private:
        // implement GPS reading here
        void callback() override;

    public:
        GPSSensorThread() : SensorThread() {}
};

#endif

