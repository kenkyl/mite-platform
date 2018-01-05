#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <stdio.h>

const unsigned int ledPin = 14; 

int ledLow = 0, ledHigh = 255; 

void setup() {
  Serial.begin(115200);
  delay(100);

  // initialize LED pin
  pinMode(ledPin, OUTPUT); 
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(ledPin, ledHigh); 
  delay(500); 
  analogWrite(ledPin, ledLow); 
  delay(500); 

}
