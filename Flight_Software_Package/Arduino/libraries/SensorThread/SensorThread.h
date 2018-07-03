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

        String getSensorName() {
            return sensorName;
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

class GeigerSensorThread : public SensorThread {
    private:
        void readFromSensor() override;
        static void ISR1();
        static void ISR2();

    private:
        static volatile int m_interruptPin[2];
        static volatile uint16_t m_eventCount[3];
        static volatile unsigned long m_eventTime[2];

    public:
	    GeigerSensorThread(int interruptPin1, int interruptPin2);

};  

#endif

