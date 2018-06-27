void setup() {
  // put your setup code here, to run once:
  Serial1.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available()) {
    char gps_string = Serial1.read();
    Serial.write(gps_string);
  }
}
