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
	sensorData = "";
	if (Serial1.available() > 0) 
	{
		while(Serial1.available() > 0)
		{
		  sensorData.concat((char)Serial1.read());  
		}

    sensorData.trim();
	sensorData.replace("\n","");
	sensorData.replace("\r","");
	
	// This code works correctly but right at this point the string in sensorData looks like:
	// $GPGGA,,4413.41602,N,07629.96370,W,7,00,,,,,,,*4A$GPVTG,,,,,,,,,M*33$GPGGA,,4413.41602,N,07629.96370,W,7,00,,,,,,,*4A$GPVTG,,,,,,,,,M*33$GPGGA,,4413.41602,
	// A line like this needs to be refined down. Look up the meanings of the different NMEA lines, put code below which extracts only the data we want, 
	// and reformat the data into something like:
	// lat,lon,alt, ... etc
	// finally, update the header information in 
	//public:
    //    GPSSensorThread() : SensorThread("GPS", <header here>) {}
	// to reflect the new format.
	
	}
}

void IMUSensorThread::readFromSensor() {
    // Put IMU data acquisition code here
    
    // Save the output data in sensorData
    sensorData = "IMU data";
}

