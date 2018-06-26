// This uses the ArduinoThread lib
#include <Thread.h>

//Instantiate Thread
Thread myThread = Thread();

// callback for myThread
void Callback(){
  Serial.print("Thread: ");
  Serial.println(millis());
}

void setup(){
  Serial.begin(9600);
  myThread.onRun(Callback);
  myThread.setInterval(500);
}

void loop(){
  // checks if thread should run
  if(myThread.shouldRun())
    myThread.run();

  // Other code...
  Serial.print("Main: ");
  Serial.println(millis());
  delay(100);
}
