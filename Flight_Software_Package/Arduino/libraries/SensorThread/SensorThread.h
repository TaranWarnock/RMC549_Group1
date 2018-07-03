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
        GPSSensorThread() : SensorThread("GPS", "UTC,Latitude(DegMin),Longitude(DegMin),Altitude(m),GroundSpeed(km/sec)") {}
};

class IMUSensorThread : public SensorThread {
    private:
        Adafruit_BNO055* bnoPtr;    // pointer to sensor handler

        // Function for a single IMU reading
        void readFromSensor() override;
		
		String getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type, String title);
		String displayCalStatus(void);

    public:
        IMUSensorThread(Adafruit_BNO055* bno) : SensorThread(
                "IMU", 
                "Accelerometer_x(m/s^2),Accelerometer_y(m/s^2),Accelerometer_z(m/s^2),"
                "Gyroscope_x(rad/s),Gyroscope_y(rad/s),Gyroscope_z(rad/s),"
                "Magnetometer_x(uT),Magnetometer_y(uT),Magnetometer_z(uT),"
                "Euler_x(deg),Euler_y(deg),Euler_z(deg),"
                "LinearAcceleration_x(m/s^2),LinearAcceleration_y(m/s^2),LinearAcceleration_z(m/s^2),"
                "Gravity_x(m/s^2),Gravity_y(m/s^2),Gravity_z(m/s^2),"
                "Temperature(C),SystemCalibration(0-3),GyroCalibration(0-3),"
                "AccelerometerCalibration(0-3),MagnetometerCalibration(0-3)") {
            bnoPtr = bno;
        }
};

class GeigerSensorThread : public SensorThread {
    private:
        void readFromSensor() override;
        static void ISR1();
        static void ISR2();

    public:
        static volatile int m_interruptPin[2];
        static volatile uint16_t m_eventCount[3];
        static volatile unsigned long m_eventTime[2];

    public:
	    GeigerSensorThread(int interruptPin1, int interruptPin2);

};  

#endif

