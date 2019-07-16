#include <SPI.h>
#include <SD.h>

const byte intPin[] = {10, 11, 12, 13};
uint32_t eventCount[] = {0, 0};
unsigned int eventTime[1000] = { 0 };
int timearrayctr = 0; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(intPin[0], INPUT_PULLUP);
  pinMode(intPin[1], INPUT_PULLUP);
  pinMode(intPin[2], INPUT_PULLUP);
  pinMode(intPin[3], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin[0]), eventDetectGeiger1, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPin[1]), eventNoiseGeiger1, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPin[2]), eventDetectGeiger2, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPin[3]), eventNoiseGeiger2, FALLING);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
} 

void loop()
{
  Serial.print(eventCount[0]);
  Serial.print(" ");
  // Serial.print(eventTime[0]);
  // Serial.print(" ");
  Serial.print(eventCount[1]);
  Serial.print(" ");
  // Serial.println(eventTime[1]);
  Serial.println(eventTime[0]);
  delay(2000);
  timearrayctr = 0;

    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    for (int j = 0; j < 999; j++) {
        dataFile.print(eventTime[j]);
        dataFile.print(" ");
        eventTime[j] = 0;
        
    }
    dataFile.print('\n');
    
    dataFile.close();
    // print to the serial port too:
    // Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
}
}


void eventDetectGeiger1()
{
  eventCount[0]++;
  eventTime[timearrayctr] = millis();
  timearrayctr++;
  // Serial.print(eventTime[0]);
  // Serial.print("1: ");
  // Serial.print(eventCount[0]);
  // Serial.print(" ");
  // Serial.println(eventTime[0] - eventTime[1]);
}

void eventNoiseGeiger1()
{
  eventCount[0]--;
  timearrayctr--;
  //eventCount[1]++;
  // eventTime[0] = micros();
  Serial.print("Noise Detected from Geiger 1, removing count");
}

void eventDetectGeiger2()
{
  eventCount[1]++;
  eventTime[timearrayctr] = millis();
  timearrayctr++;
  // Serial.print("2: ");
  // Serial.print(eventCount[1]);
  // Serial.print(" ");
  // Serial.println(eventTime[1] - eventTime[0]);
}

void eventNoiseGeiger2()
{
  eventCount[1]--;
  timearrayctr--;
  Serial.print("Noise Detected from Geiger 2, removing count");
}
