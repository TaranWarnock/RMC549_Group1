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

// create handler for IMU
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// create thread for each sensor
SensorThread* emu_thread = new EmuSensorThread();
SensorThread* gps_thread = new GPSSensorThread();
SensorThread* imu_thread = new IMUSensorThread(&bno);
SensorThread* geiger_thread = new GeigerSensorThread(11, 12);

// create controller to hold the threads
ThreadController controller = ThreadController();

void setup() {
  // put your setup code here, to run once:
  // ------------------------ Changing the baud rate of GPS Serial1 port ---------------------------
  Serial1.begin(4800);
  Serial1.write("$PTNLSPT,115200,8,N,1,4,4*11\r\n");
  delay(1000);
  Serial1.end();
  // -----------------------------------------------------------------------------------------------
  
  Serial1.begin(115200); // Setting up GPS serial com
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the IMU
  doIMU = bno.begin();
  if(!doIMU)
  {
    // could send an error message to the Pi here
  }

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
  controller.add(emu_thread);
  controller.add(gps_thread);
  controller.add(imu_thread);
  controller.add(geiger_thread);
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
      full_data.concat("ArdTimeStamp");

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
      full_data.concat("ms");
      
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
      String dataStr = pi_command.substring(2);  // remove TX from string, might not want to do this

      int len_data = dataStr.length() + 1;
      char radiopacket[len_data];
      dataStr.toCharArray(radiopacket, len_data);

      rf95.send((uint8_t *)radiopacket, len_data);
      rf95.waitPacketSent();

      // send OK to the Pi
      Serial.println("OK");
    }
    
    else
    {
      // Do nothing. Pi will know of error on timeout.
    }
  }
}
