/*
 * file: SensorThread.h
 *
 * The SensorThread class provides a base for adding new sensor implementations.
 * SensorThread extends the arduino Thread class developed by Ivan Seidel at
 * https://github.com/ivanseidel/ArduinoThread
 *
 * The threads are actually "pseudo" threads; they do not run in parallel 
 * because the arduino is not capable of multithreading. Each thread is
 * run sequentially until its task is complete. The main benefit of the
 * "thread" interface is to provide a framework for easily adding new
 * sensors to the payload.
 *
 * To add a new sensor the steps are:
 *  1. Create a new class which extends the SensorThread class
 *  2. In the constructor of the new class, call the SensorThread
 *     constructor with the following arguments:
 *       a. name :: String containing name of sensor
 *       b. header :: String containing all values returned by
 *            sensor, separated by commas
 *  3. Override the readFromSensor() method. In this method, read
 *       from the sensor and save the measured data into the
 *       sensorData variable, in a comma-separated format which
 *       matches the header argument passed to the constructor.
 *
 * The process for obtaining a data sample from a single sensor is to call the
 *   run() method then use the getSensorData() method to read the data.
 */

#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>             // Arduino threading library
#include <Adafruit_BNO055.h>    // IMU sensor library
#include <utility/imumaths.h>   // used in IMU code to create vector objects

class SensorThread : public Thread {
    protected:
        String sensorName;
        String sensorHeader;
        String sensorData;      // save sensor data here

        // Function for a single sensor reading event. This is a pure virtual function
        // which specific sensor classes will provide the implementation for.
        virtual void readFromSensor()  = 0;

    public:
        // constructor should set sensor name and establish data header (names of data vars in order)
        SensorThread() : Thread() {}
        SensorThread(String name, String header) : Thread() {
            sensorName   = name;
            sensorHeader = header;
        }

        // function which is called when the thread is started
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


class GPSSensorThread : public SensorThread {
    private:
//        bool active = true; // variable to decide if GPS is working
        int timeCheck = 8000;   // milliseconds

        // Function for a single GPS reading
        void readFromSensor() override;

    public:
        GPSSensorThread() : SensorThread("GPS", "UTC,LtDgMn,NS,LnDgMn,EW,Nsat,Alt,Altu") {}
};


class IMUSensorThread : public SensorThread {
    private:
        Adafruit_BNO055* bnoPtr;    // pointer to IMU handler

        bool IMUactive = true, pressureActive = true;

        // Function for a single IMU reading
        void readFromSensor() override;

        /* Returns a 3D vector quantity measured by the IMU, as a string of three comma-separated values.
         *    sensor_type -- should be one of:
         *     Adafruit_BNO055::VECTOR_ACCELEROMETER
         *     Adafruit_BNO055::VECTOR_GYROSCOPE
         *     Adafruit_BNO055::VECTOR_MAGNETOMETER
         *     Adafruit_BNO055::VECTOR_EULER
         *     Adafruit_BNO055::VECTOR_LINEARACCEL
         *     Adafruit_BNO055::VECTOR_GRAVITY
         *   tile -- Previously, this string would be included in the returned vector.
         *          It is no longer included in the output but is left here to avoid
         *          breaking other code.
         */
        String getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type, String title);
        
        // Returns the calibration status of the IMU as a String of four comma-separated integers,
        //   with each integer indicating the calibration status with a value from 0-3.
        String displayCalStatus(void);

    public:
        IMUSensorThread(Adafruit_BNO055* bno) : SensorThread(
                "IMU,Pr",
                "Acxms2,Acyms2,Aczms2,"
                "Gyxrs,Gyyrs,Gyzrs,"
                "MgxuT,MgyuT,MgzuT,"
                "Elxdg,Elydg,Elzdg,"
                "LAcxms2,LAcyms2,LAczms2,"
                "Gvxms2,Gvyms2,Gvzms2,"
                "TC,SyCl03,GyCl03,"
                "AcCl03,MgCl03") {
            bnoPtr = bno;
        }
};

class GeigerSensorThread : public SensorThread {
    private:
        // function which saves the number of geiger counts since the previous sample
        void readFromSensor() override;

        // Interrupt Service Routines which are called every time a new count occurs.
        // There is one for each geiger counter.
        static void ISR1();
        static void ISR2();
        static void ISR3();
        static void ISR4();

    public:
        // These variables must be static and volatile because they are used by
        // the interrupt service routines
        static volatile int m_interruptPin[4];          // pins which trigger interrupts
        static volatile uint16_t m_eventCount[3];       // number of counts since last sample
        static volatile unsigned int m_eventTime[101];   // timestamp of most recent count
        static volatile int m_timearrayctr[1];

    public:
        GeigerSensorThread(int interruptPin1, int interruptPin2, int interruptPin3, int interruptPin4);

};

#endif

