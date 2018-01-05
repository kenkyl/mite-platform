#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <stdio.h>

const char* ssid     = "connectory 2";
const char* password = "c0nn3ct0ry2";

const unsigned int ledPin = 5; 
const unsigned int buzPin = 4; 

int ledLow = 0, ledHigh = 255; 

const unsigned int melodyFreq = 784; 


WiFiUDP Udp; 
unsigned int localUdpPort = 4211; 
unsigned int remoteUdpPort = 5000; 
IPAddress remoteIp(10,10,90,249); 

char incomingPacket[55]; 

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

  // subsrcibe 
  char subPacket[] = "subscribe"; 
  Udp.beginPacket(remoteIp, remoteUdpPort); 
  Udp.write(subPacket); 
  Udp.endPacket(); 
}

void loop() {
  // put your main code here, to run repeatedly:

  int packetSize = Udp.parsePacket(); 
  if(packetSize) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 55);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    // convert to float and check 
    float value = (float)atof(incomingPacket); 
    check(value); 
  }
}

void check(float value) {
  float limit = 1.5;
  // WRITE IN LOGIC HERE 
  if (value < limit) {
    Serial.printf("ALERT: Value below limit\n"); 
    alert(); 
  }
}

void alert() {
  // turn light and buzzer on 
  noTone(buzPin); 
  analogWrite(ledPin, ledHigh);
  tone(buzPin, melodyFreq); 

  delay(2000); 

  // turn light and buzzer off
  analogWrite(ledPin, ledLow); 
  noTone(buzPin); 
}

