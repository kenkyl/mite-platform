#include <ESP8266WiFi.h>
#include <stdio.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/***************** Pins *****************/
#define PIN_PC    0
/****************************************/

/*********** WIFI Credentials ***********/
#define WLAN_SSID   "connectory 2"
#define WLAN_PASS   "c0nn3ct0ry2"
/****************************************/

/********** Adafruit.io Setup  **********/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883               // use 8883 for SSL
#define AIO_USERNAME    "connectory_io"
#define AIO_KEY         "76805a2a35c84583a9ac77cd9ec5f546"
/****************************************/

/********** Global State ****************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************************/

/*************** Feeds ******************/
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
/****************************************/

void setup() {
  Serial.begin(9600);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
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

  // would add subscribe here

}

int sensorValue;
float voltage; 

void loop() {
  // establish/ensure connection 
  MQTT_connect(); 

  // delay between connect and publish 
  delay(2000);

  // collect and adjust photcell value
  sensorValue = analogRead(PIN_PC); 
  voltage = sensorValue * (5.00 / 1023.00);

  // publish 
  Serial.print(F("Sending photocell val: "));
  Serial.print(voltage); 
  Serial.print("..."); 
  if (! photocell.publish(voltage)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
