#include <ESP8266WiFi.h>
#include <stdio.h>
#include <string.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/***************** Pins *****************/
#define PIN_LED     5
#define PIN_BUZ     4
/****************************************/

/**************** Other *****************/
#define LED_HIGH      255
#define LED_LOW       0
#define BUZ_FREQ      784
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
// Setup a feed called 'onoffbutton' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/on_off");
/****************************************/

void setup() {
  Serial.begin(115200);
  delay(100);

  // initialize LED pin
  pinMode(PIN_LED, OUTPUT); 

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

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) {
        analogWrite(PIN_LED, LED_LOW);
      }
      else if (strcmp((char *)onoffbutton.lastread, "ON") == 0) {
        analogWrite(PIN_LED, LED_HIGH);
      }
    }
  }
}

/*
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
*/

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

