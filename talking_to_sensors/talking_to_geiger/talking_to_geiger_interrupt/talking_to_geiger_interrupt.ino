const byte intPin[] = {11, 12};
uint32_t eventCount[] = {0, 0};
unsigned long eventTime[] = {0, 0};

void setup() {
  // put your setup code here, to run once:
  pinMode(intPin[0], INPUT);
  pinMode(intPin[1], INPUT);
  attachInterrupt(digitalPinToInterrupt(intPin[0]), eventDetectGeiger1, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPin[1]), eventDetectGeiger2, FALLING);
}

void loop()
{
  Serial.print(eventCount[0]);
  Serial.print(" ");
  Serial.print(eventTime[0]);
  Serial.print(" ");
  Serial.print(eventCount[1]);
  Serial.print(" ");
  Serial.println(eventTime[1]);
  delay(1000);
}


void eventDetectGeiger1()
{
  eventCount[0]++;
  eventTime[0] = micros();
//  Serial.print("1: ");
//  Serial.print(eventCount[0]);
//  Serial.print(" ");
//  Serial.println(eventTime[0] - eventTime[1]);
}

void eventDetectGeiger2()
{
  eventTime[1] = micros();
  eventCount[1]++;
//  Serial.print("2: ");
//  Serial.print(eventCount[1]);
//  Serial.print(" ");
//  Serial.println(eventTime[1] - eventTime[0]);
}

