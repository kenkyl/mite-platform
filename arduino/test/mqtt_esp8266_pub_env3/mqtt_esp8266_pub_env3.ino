#include <ESP8266WiFi.h>
#include <stdio.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

/***************** Pins *****************/
#define PIN_PC          0
#define PIN_DHT          4
/****************************************/

/**************** Other *****************/
#define DHTTYPE         DHT22
/****************************************/

/*********** WIFI Credentials ***********/
#define WLAN_SSID       "connectory 2"
#define WLAN_PASS       "c0nn3ct0ry2"
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

// define DHT object
DHT dht(PIN_DHT, DHTTYPE); 
/****************************************/

/*************** Feeds ******************/
// Setup publishing feeds 
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
/****************************************/

void setup() {
  Serial.begin(9600);
  delay(100);

  // Wait for serial to initialize.
  while(!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");

  // connect WiFi
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
  Serial.println("-------------------------------------");

  // initialize DHT
  dht.begin(); 
  
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  delay(100); 
}

int sensorValue;
int timeSinceLastRead[2] = {0, 0}; 
float voltage; 

void loop() {
  // establish/ensure connection 
  MQTT_connect(); 

/*
  // collect and publish photocell value every 2 seconds 
  if(timeSinceLastRead[0] > 2000) {
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
    timeSinceLastRead[0] = 0; 
  }
  */
  // collect and publish humidity and temp values every 9 seconds 
  if(timeSinceLastRead[1] > 5000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead[1] = 0;
      return;
    }

    // publish humidity 
    Serial.print(F("Sending humidity val: "));
    Serial.print(h); 
    Serial.print("%..."); 
    if (! photocell.publish(h)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    
    // delay one second to avoid data limit throttle 
    delay(1000); 

    // publish temp 
    Serial.print(F("Sending temp val: "));
    Serial.print(t); 
    Serial.print("*C..."); 
    if (! photocell.publish(t)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }

    // adjust time counters in accordance with delay 
    timeSinceLastRead[0] += 1000; 
    timeSinceLastRead[1] = 1000; 
  }

  delay(100); 
  // increment time counters
  timeSinceLastRead[0] += 100; 
  timeSinceLastRead[1] += 100; 
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
