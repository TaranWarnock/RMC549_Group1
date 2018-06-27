#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
  
Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Orientation Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);
}

void getvec(Adafruit_BNO055::adafruit_vector_type_t sensor_type, char* title){
  imu::Vector<3> data_vector = bno.getVector(sensor_type);
  Serial.print(title);
  Serial.print(": X: ");
  Serial.print(data_vector[0]);
  Serial.print("  Y: ");
  Serial.print(data_vector[1]);
  Serial.print("  Z: ");
  Serial.print(data_vector[2]);
  Serial.println("");
}

void loop(void) 
{
  getvec(Adafruit_BNO055::VECTOR_ACCELEROMETER, "Acceleration");
  getvec(Adafruit_BNO055::VECTOR_GYROSCOPE, "Gyroscope");
  getvec(Adafruit_BNO055::VECTOR_MAGNETOMETER , "Magnetometer");
  getvec(Adafruit_BNO055::VECTOR_EULER, "Euler");
  getvec(Adafruit_BNO055::VECTOR_LINEARACCEL, "LinearAccel");
  getvec(Adafruit_BNO055::VECTOR_GRAVITY, "Gravity");
  // get temperature and print to consol (accuracy of sensor is 1 degree)
  int temp = bno.getTemp();
  Serial.print("\tCurrent Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  
  delay(1500);
}
