#include <SensorThread.h>
#include <ThreadController.h>

// create handler for IMU
Adafruit_BNO055 bno = Adafruit_BNO055(55);
MPL3115A2 preassure;

// create thread for each sensor
SensorThread* gps_thread = new GPSSensorThread();
SensorThread* imu_thread = new IMUSensorThread(&bno, &preassure);
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
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    //Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    //while(1);
  }

  preassure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  preassure.setOversampleRate(7); // Set Oversample to the recommended 128
  preassure.enableEventFlags(); // Enable all three pressure and temp event flags 

  // add each thread to the controller
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
    
      // Uncomment if data must be a character array:
      //size_t bufsize = 256;
      //char data_buffer[bufsize];
      //full_data.toCharArray(data_buffer, bufsize);
      //Serial.println(data_buffer);  // DEBUG statement
    }
    else
    {
      // Do nothing. Pi will know of error on timeout.
    }
  }
}
