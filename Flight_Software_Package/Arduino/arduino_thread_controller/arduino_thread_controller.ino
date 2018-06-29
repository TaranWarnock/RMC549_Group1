#include <SensorThread.h>
#include <ThreadController.h>

// create thread for each sensor
SensorThread* gps_thread = new GPSSensorThread();
SensorThread* imu_thread = new IMUSensorThread();

// create controller to hold the threads
ThreadController controller = ThreadController();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // add each thread to the controller
  controller.add(gps_thread);
  controller.add(imu_thread);
}

void loop() {
  // put your main code here, to run repeatedly:

  // wait for instruction from Pi
 
  // this will instruct each sensor thread to measure and save its data
  controller.run();
  
  String full_data; // full data array to send to Pi
  String temp_data; // temporary data holder for each sensor

  // get the current time
  unsigned long time = millis();
  full_data.concat(time);
  full_data.concat("ms: ");
  
  for (int i = 0; i < controller.size(); i++) {
    // get sensor data
    temp_data = ((SensorThread*) controller.get(i))->getSensorData();
    // append to final data output
    full_data.concat(temp_data);
  }

  // Uncomment if data must be a character array:
  //size_t bufsize = 256;
  //char data_buffer[bufsize];
  //full_data.toCharArray(data_buffer, bufsize);
  //Serial.println(data_buffer);  // DEBUG statement

  // Debugging statement to print data and wait 1 sec
  Serial.println(full_data);
  delay(1000);
}
