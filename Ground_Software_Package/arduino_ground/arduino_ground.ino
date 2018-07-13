// Author: Curtis Puetz
// Adapted from: https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/rfm9x-test

#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 5
#define RFM95_RST 6
#define RFM95_INT 9

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED LED_BUILTIN

void setup() {
  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void loop(){
	// check if there is communication from the LoRa device (i.e. check if downlinked data has been recieved downlinked data is recieved)
  if (rf95.available()){
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)){
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      // wait for a sufficient amount of time for the full data string to be recieved
      delay(5); 
      // print the recieved data on the serial port with the RSSI signal strength appended to it
      Serial.print((char*)buf);
      Serial.print(",");
      Serial.println(rf95.lastRssi(), DEC);
    }
    else{
      Serial.println("Receive failed");
    }
  }
  // check if commands from the main ground station computer have been recieved (for uplink to baloon)
  if (Serial.available() > 0){
  String data = "";
  // read the complete uplink command
  while (Serial.available()>0){
    data.concat((char)Serial.read());
  }
  int len_data = data.length() + 1;
  char data1[len_data];
  data.toCharArray(data1, len_data);
  // send the uplink command
  rf95.send((uint8_t*)data1, len_data);
  rf95.waitPacketSent();
  //Serial.println("Sent a reply");
  // write the LED low for visual confirmation that the command has been sent
  digitalWrite(LED, LOW);
  }
}
  

 
