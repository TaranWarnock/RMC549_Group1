void setup() {
  // put your setup code here, to run once:
  Serial1.begin(4800);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
