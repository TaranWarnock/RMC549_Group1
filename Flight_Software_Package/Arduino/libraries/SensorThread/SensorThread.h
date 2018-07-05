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
        GPSSensorThread() : SensorThread("GPS", "UTC,Lt_DgMn,Ln_DgMn,Alt_m,GS_km/s") {}
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
                "IMU,Pr",
                "Acx_m/s2,Acy_m/s2,Acz_m/s2,"
                "Gyx_rad/s,Gyy_rad/s,Gyz_rad/s,"
                "Mgx_uT,Mgy_uT,Mgz_uT,"
                "Elx_dg,Ely_dg,Elz_dg,"
                "LAcx_m/s2,LAcy_m/s2,LAcz_m/s2,"
                "Gvx_m/s2,Gvy_m/s2,Gvz_m/s2,"
                "T_C,SyCl_0-3,GyCl_0-3,"
                "AcCl_0-3,MgCl_0-3,Pr_Pa,TPr_C") {
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

