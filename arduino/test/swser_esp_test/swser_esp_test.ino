
#include <SoftwareSerial.h>
#include <stdio.h>
#include <string.h>

#define rxPIN 14
#define txPIN 12 

SoftwareSerial swSer(rxPIN, txPIN, false, 256);

void setup() {
  // define pin modes for tx, rx:
  pinMode(rxPIN, INPUT);
  pinMode(txPIN, OUTPUT);
  Serial.begin(115200);
  
  swSer.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("\nSoftware serial RECV test started");

//  for (char ch = ' '; ch <= 'z'; ch++) {
//    swSer.write(ch);
//  }
//  swSer.println("");

}

void loop() {
//delay(100); 
//  while (swSer.available() > 0) {
//    Serial.println(swSer.read());
//    //Serial.println("send"); 
//  }
//  while (Serial.available() > 0) {
//    swSer.write(Serial.read());
//    Serial.println("recv"); 
//  }
//  Serial.println("loop"); 

  char incomingSerialData[32];
  getCommand(incomingSerialData);
//  Serial.println("--");
//  Serial.print(incomingSerialData);
//  Serial.print("\n");
  
}

void getCommand(char *incomingSerialData)
{
  int incomingSerialDataIndex = 0; // Stores the next 'empty space' in the array

  while(swSer.available() > 0)
  {
    incomingSerialData[incomingSerialDataIndex] = swSer.read();
    if (incomingSerialData[incomingSerialDataIndex] != 13) {
      incomingSerialDataIndex++; // Ensure the next byte is added in the next position
    }
    incomingSerialData[incomingSerialDataIndex] = '\0';
  }

  if(incomingSerialDataIndex > 0) {
    Serial.print(incomingSerialData);
    Serial.print("\n");
  }
}

void parseCommand(char *sensorMessage)
{
  int stringPos = 0; 
  int splitCount = 0; 

  while(stringPos < strlen(sensorMessage)) {
    
  }
}


