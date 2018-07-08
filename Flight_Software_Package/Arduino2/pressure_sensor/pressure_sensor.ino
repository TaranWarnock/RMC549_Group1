#include "SparkFunMPL3115A2.h"

// create handler for pressure sensor
MPL3115A2 preSensor;

unsigned long sampleTime = 1000;


#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  while (!SD.begin(4)) {
    delay(1000);
  }
  // pressure sensor init
  preSensor.begin();
  preSensor.setModeBarometer();   // Measure pressure in Pascals from 20 to 110 kPa
  preSensor.setOversampleRate(7); // Set Oversample to the recommended 128
  preSensor.enableEventFlags();   // Enable all three pressure and temp event flags 
}

void loop() {
  String sensorData = "";
  // get the time
  unsigned long timeStamp = millis();
  sensorData.concat(timeStamp);
  sensorData.concat(",");

  // read in values
  sensorData.concat(preSensor.readPressure());
  sensorData.concat(",");
  sensorData.concat(preSensor.readTemp());

  // save to SD card
  myFile = SD.open("PrDa.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(sensorData);
    myFile.close();
  } 
  delay(sampleTime);
}
