#include <SensorThread.h>
#include <ThreadController.h>
#include <RH_RF95.h>

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
bool doPhoto;

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
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the IMU
  doIMU = bno.begin();
  if(!doIMU)
  {
    // could send an error message to the Pi here
  }

  // Initialize photosensor 0
  doPhoto = tsl0.begin();
  if(!doPhoto) {
    // error?
  }
  tsl0.setGain(TSL2561_GAIN_0X);                 // set no gain (for bright situations)
  tsl0.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  // Initialize 2nd photosensor 1
  doPhoto = doPhoto && tsl1.begin();
  if(!doPhoto) {
    // error?
  }
  tsl1.setGain(TSL2561_GAIN_0X);                 // set no gain (for bright situations)
  tsl1.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  // Initialize the photosensor 2
  doPhoto = doPhoto && tsl2.begin();
  if(!doPhoto) {
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
}

void loop() {
  String full_data;  // full data array to send to Pi
  String temp_data;  // temporary data holder for each sensor
  String pi_command; // command from Pi to do something

  String deviceName = "RockyFeatherOne";

  // wait for instruction from Pi
  if (Serial.available() > 0)
  {
    while(Serial.available() > 0)
    {
      pi_command.concat((char)Serial.read());  
    }

    pi_command.trim();
    pi_command.replace("\n","");
    pi_command.replace("\r","");
    
    if (pi_command.equalsIgnoreCase("ID"))
    {
      // Pi has asked for arduino ID and sensor(s) ID.

      full_data.concat(deviceName);

      for (int i = 0; i < controller.size(); i++) 
      {
        // get sensor ID
        temp_data = ((SensorThread*) controller.get(i))->getSensorName();
        // append to final data output
        full_data.concat(",");
        full_data.concat(temp_data);
      }

      // Send data to Pi
      Serial.println(full_data);
    }

    else if (pi_command.equalsIgnoreCase("HEADER"))
    {
      // Pi has asked for data headers
      full_data.concat("ATSms");

      for (int i = 0; i < controller.size(); i++) 
      {
        // get sensor ID
        temp_data = ((SensorThread*) controller.get(i))->getSensorHeader();
        // append to final data output
        full_data.concat(",");
        full_data.concat(temp_data);
      }

      // Send data to Pi
      Serial.println(full_data);
    }

    else if (pi_command.equalsIgnoreCase("DATA"))
    {
      // Pi has asked for new sensor data 
      // this will instruct each sensor thread to measure and save its data
      controller.run();
    
      // get the current time
      unsigned long time = millis();
      full_data.concat(time);
      
      for (int i = 0; i < controller.size(); i++) {
        // get sensor data
        temp_data = ((SensorThread*) controller.get(i))->getSensorData();
        // append to final data output
        full_data.concat(",");
        full_data.concat(temp_data);
      }

      // Send data to Pi
      Serial.println(full_data);
    }
    
    else if ((pi_command.substring(0, 2)).equalsIgnoreCase("TX"))
    {
      // Pi has asked to transmit data to ground

      if (doTelemetry) {
      
        String dataStr = pi_command.substring(2);  // remove TX from string, might not want to do this

        int len_data = dataStr.length() + 1;
        char radiopacket[len_data];
        dataStr.toCharArray(radiopacket, len_data);

        rf95.send((uint8_t *)radiopacket, len_data);
        rf95.waitPacketSent();
      }
      // send OK to the Pi
      Serial.println("OK");
    }

    else if ((pi_command.substring(0, 2)).equalsIgnoreCase("RX"))
    {
      // Pi has asked for commands from ground
      // Send the command list
      Serial.println(ground_commands);
      // Clear the command list
      ground_commands = ""; 
    }
    
    else
    {
      // Do nothing. Pi will know of error on timeout.
    }
  }

  // Check for commands from ground
  if (doTelemetry && rf95.available()) {
    // Receive the message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      // save to command list
      ground_commands.concat(String((char*)buf));
    }
    ground_commands.concat(String(rf95.lastRssi()));
    ground_commands.concat(",");
  }
  
}
