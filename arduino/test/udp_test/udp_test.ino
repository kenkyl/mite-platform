#include <ESP8266WiFi.h>
#include <WifiUdp.h>

const char* ssid     = "connectory 2";
const char* password = "c0nn3ct0ry2";

WiFiUDP Udp; 
unsigned int localUdpPort = 4210; 
unsigned int remoteUdpPort = 5000; 
IPAddress remoteIp(10,10,90,249); 
char packetMessage[] = "Testing, testing, 123...";
 
void setup() {
  Serial.begin(115200);
  delay(100);

  // We start by connecting to a WiFi network

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

void loop() {
  delay(5000);
  // send udp packet
  Udp.beginPacket(remoteIp, remoteUdpPort); 
  Udp.write(packetMessage); 
  Udp.endPacket(); 
  
}
