#include "SparkFunMPL3115A2.h"

// create handler for pressure sensor
MPL3115A2 preSensor;

unsigned long sampleTime = 1000;


#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("Init Success");
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
    Serial.print("Writing to test.txt...");
    myFile.println(sensorData);
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening PressureData.txt");
  }
   delay(sampleTime);
}
