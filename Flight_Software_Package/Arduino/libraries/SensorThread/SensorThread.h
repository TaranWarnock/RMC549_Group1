#ifndef SENSORTHREAD_H
#define SENSORTHREAD_H

#include <Thread.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>   // used in IMU code
#include "SparkFunMPL3115A2.h"

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


class GPSSensorThread : public SensorThread {
    private:
        // Function for a single GPS reading
        void readFromSensor() override;

    public:
        GPSSensorThread() : SensorThread("GPS", "UTC,Lat(DegMin),Lon(DegMin),Alt(m),GS(km/sec)") {}
};

class IMUSensorThread : public SensorThread {
    private:
        Adafruit_BNO055* bnoPtr;    // pointer to sensor handler
        MPL3115A2* preassurePtr;    // pointer to preassure

        // Function for a single IMU reading
        void readFromSensor() override;

                String getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type, String title);
                String displayCalStatus(void);

    public:
        IMUSensorThread(Adafruit_BNO055* bno, MPL3115A2* preassure) : SensorThread(
                "IMU,Pres",
                "Acc_x(m/s^2),Acc_y(m/s^2),Acc_z(m/s^2),"
                "Gyro_x(rad/s),Gyro_y(rad/s),Gyro_z(rad/s),"
                "Mag_x(uT),Mag_y(uT),Mag_z(uT),"
                "Eul_x(deg),Eul_y(deg),Eul_z(deg),"
                "LinAcc_x(m/s^2),LinAcc_y(m/s^2),LinAcc_z(m/s^2),"
                "Grav_x(m/s^2),Grav_y(m/s^2),Grav_z(m/s^2),"
                "Temp(C),SysCal(0-3),GyroCal(0-3),"
                "AccCal(0-3),MagCal(0-3),Pres(Pa),TempPres(C)") {
            bnoPtr = bno;
            preassurePtr = preassure;
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

