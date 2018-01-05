#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <stdio.h>

const char* ssid     = "connectory 2";
const char* password = "c0nn3ct0ry2";

const unsigned int sensorPin = 0; 
const unsigned int ledPin = 14; 

int lightLevel, ledLow = 0, ledHigh = 255, lightLow = 0, lightHigh = 1023; 

WiFiUDP Udp; 
unsigned int localUdpPort = 4210; 
unsigned int remoteUdpPort = 5000; 
IPAddress remoteIp(10,10,90,249); 
char packetHeader[] = "Light value: ";
 
void setup() {
  Serial.begin(115200);
  delay(100);

  // initialize LED pin
  pinMode(ledPin, OUTPUT); 

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  Udp.begin(localUdpPort); 
  
}

int sensorValue;
float voltage; 
//char* packetMessage; 

char pDeviceId[8];
char pLabel[16];
char pType[8]; 

void loop() {
  delay(500);
  
  // get sensor data and convert 
  sensorValue = analogRead(sensorPin); 
  voltage = sensorValue * (5.00 / 1023.00); 

  // append voltage to message
  char pValue[16];
  dtostrf(voltage, 16, 3, pValue); 
  Serial.println(voltage);

  // adjust lightLevel
  manualTune(); 

  // turn LED on while sending 
  analogWrite(ledPin, sensorValue); 

  // set values 
  strncpy(pDeviceId, "S01", 8); 
  strncpy(pLabel, "light test", 16);
  strncpy(pType, "light", 8);  
  
  // send udp packet
  char packet[55] = "";   // extra for null character 
  strncat(packet, pDeviceId, 8);
  strcat(packet, "::"); 
  strncat(packet, pType, 8); 
  strcat(packet, "::"); 
  strncat(packet, pLabel, 16);
  strcat(packet, "::");  
  strncat(packet, pValue, 16); 
  packet[48] = '\0'; 

  // SEND IT 
  Serial.println(pValue); 
  Serial.println(packet);
  Udp.beginPacket(remoteIp, remoteUdpPort); 
  Udp.write(packet); 
  Udp.endPacket(); 

  // turn LED off after send
  delay(100); 
  analogWrite(ledPin, ledLow); 
}

void manualTune() {
  sensorValue = map(sensorValue, lightLow, lightHigh, ledLow, ledHigh); 
  sensorValue = constrain(sensorValue, ledLow, ledHigh); 
}

