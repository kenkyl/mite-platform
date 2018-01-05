#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <stdio.h>

const char* ssid     = "connectory 2";
const char* password = "c0nn3ct0ry2";

WiFiUDP Udp; 
unsigned int localUdpPort = 4210; 
unsigned int remoteUdpPort = 5000; 
IPAddress remoteIp(10,10,90,249); 
char packetHeader[] = "Light value: ";
 
void setup() {
  Serial.begin(115200);
  delay(100);

  // connect to Wi-Fi network 
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

  // open UDP port 
  Udp.begin(localUdpPort); 
  
}

int sensorValue;
float voltage; 

void loop() {
  delay(1000);
  
  // get sensor data and convert 
  sensorValue = analogRead(0); 
  voltage = sensorValue * (5.00 / 1023.00); 

  // append voltage to message
  char packetValue[5]; 
  dtostrf(voltage, 5, 2, packetValue); 
  //packetMessage = baseMessage + voltage; 
  Serial.println(voltage);
  
  // send udp packet
  char packet[255] = "";
  strcat(packet, packetHeader);
  strcat(packet, packetValue); 
  Serial.println(packet);
  Udp.beginPacket(remoteIp, remoteUdpPort); 
  Udp.write(packet); 
  Udp.endPacket(); 
  
}
