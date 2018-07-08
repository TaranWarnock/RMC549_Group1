#include <SensorThread.h>
#include <ThreadController.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <SD.h>

// Telemetry definitions
#define RFM95_CS 5
#define RFM95_RST 6
#define RFM95_INT 9
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Comma separated List of unprocessed commands from ground station
String ground_commands = "";

// create handler for IMU
Adafruit_BNO055 bno = Adafruit_BNO055(55);
MPL3115A2 preassure;

// light sensor
TSL2561 tsl0(TSL2561_ADDR_FLOAT);
TSL2561 tsl1(TSL2561_ADDR_LOW);
TSL2561 tsl2(TSL2561_ADDR_HIGH);

// create thread for each sensor
SensorThread* gps_thread = new GPSSensorThread();
SensorThread* imu_thread = new IMUSensorThread(&bno, &preassure);
SensorThread* geiger_thread = new GeigerSensorThread(11, 12);
SensorThread* photo_thread = new PhotoSensorThread(&tsl0, &tsl1, &tsl2);

// create controller to hold the threads
ThreadController controller = ThreadController();

bool doIMU;
bool doTelemetry;

File myFile;

void setup() {

  // put your setup code here, to run once:
  // ------------------------ Changing the baud rate of GPS Serial1 port ---------------------------
  Serial1.begin(4800);
  Serial1.write("$PTNLSPT,115200,8,N,1,4,4*11\r\n");
  delay(1000);
  Serial1.end();
  // -----------------------------------------------------------------------------------------------
  
  Serial1.begin(115200); // Setting up GPS serial com
  Serial1.write("$PTNLSCR,,,,,,,3,,*5B\r\n"); // A safety precaution to keep GPS module in the AIR mode.
  delay(1000);

  // Initialize the IMU
  doIMU = bno.begin();
  if(!doIMU)
  {
    // could send an error message to the Pi here
  }

  // Initialize photosensor 0
  if(!tsl0.begin()) {
    // error?
  }
  tsl0.setGain(TSL2561_GAIN_0X);                 // set no gain (for bright situations)
  tsl0.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  // Initialize 2nd photosensor 1
  if(!tsl1.begin()) {
    // error?
  }
  tsl1.setGain(TSL2561_GAIN_0X);                 // set no gain (for bright situations)
  tsl1.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  // Initialize the photosensor 2
  if(!tsl2.begin()) {
    // error?
  }
  tsl2.setGain(TSL2561_GAIN_0X);                 // set no gain (for bright situations)
  tsl2.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)

  preassure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  preassure.setOversampleRate(7); // Set Oversample to the recommended 128
  preassure.enableEventFlags(); // Enable all three pressure and temp event flags 

  // Initialize telemetry
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(100);
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  doTelemetry = rf95.init();
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  doTelemetry = doTelemetry && rf95.setFrequency(RF95_FREQ);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  delay(1000);

  // add each thread to the controller
  controller.add(gps_thread);
  controller.add(imu_thread);
  controller.add(geiger_thread);
  controller.add(photo_thread);

  // initialize SD card
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }

  // print the ID
  String deviceName = "RockyFeatherOne";
  String IDdata;
  String temp_data0;
  IDdata.concat(deviceName);

  for (int i = 0; i < controller.size(); i++) 
  {
    // get sensor ID
    temp_data0 = ((SensorThread*) controller.get(i))->getSensorName();
    // append to final data output
    IDdata.concat(",");
    IDdata.concat(temp_data0);
  }

  // Send data to SD card TODO!
  //Serial.println(IDdata);
  write_to_SD(IDdata);


  // print header data
  String headerData;
  String temp_data1;
  headerData.concat("ATSms");

  for (int i = 0; i < controller.size(); i++) 
  {
    // get sensor ID
    temp_data1 = ((SensorThread*) controller.get(i))->getSensorHeader();
    // append to final data output
    headerData.concat(",");
    headerData.concat(temp_data1);
  }

  // Send data to SD card TODO!
  //Serial.println(headerData);
  write_to_SD(headerData);
}

void write_to_SD(String data) {
  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println(data);
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening PressureData.txt");
  }
}

void loop() {
  String full_data;  // full data array to send to Pi
  String temp_data;  // temporary data holder for each sensor

  // save data to SD card
      // Pi has asked for new sensor data 
      // this will instruct each sensor thread to measure and save its data
      controller.run();
    
      // get the current time
      unsigned long time1 = millis();
      full_data.concat(time1);
      
      for (int i = 0; i < controller.size(); i++) {
        // get sensor data
        temp_data = ((SensorThread*) controller.get(i))->getSensorData();
        // append to final data output
        full_data.concat(",");
        full_data.concat(temp_data);
      }
      unsigned long time = millis();
      // save data to SD TODO!
      //Serial.println(full_data);
      write_to_SD(full_data);
    
   // transmit the data to the ground

        int len_data = full_data.length() + 1;
        char radiopacket[len_data];
        full_data.toCharArray(radiopacket, len_data);

        rf95.send((uint8_t *)radiopacket, len_data);
        rf95.waitPacketSent();
  
      // wait for 2 seconds to pass since the last data was taken (if it hasnt already)
      if (millis() - time > 2000){
      	delay(10);
      }

  //   else if ((pi_command.substring(0, 2)).equalsIgnoreCase("RX"))
  //   {
  //     // Pi has asked for commands from ground
  //     // Send the command list
  //     Serial.println(ground_commands);
  //     // Clear the command list
  //     ground_commands = ""; 
  //   }
    
  //   else
  //   {
  //     // Do nothing. Pi will know of error on timeout.
  //   }
  // }

  // // Check for commands from ground
  // if (rf95.available()) {
  //   // Receive the message
  //   uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  //   uint8_t len = sizeof(buf);
  //   if (rf95.recv(buf, &len)) {
  //     // save to command list
  //     ground_commands.concat(String((char*)buf));
  //   }
  //   ground_commands.concat(String(rf95.lastRssi()));
  //   ground_commands.concat(",");
  // }
  
}
