#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <stdio.h>
#include <string.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

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

/*********** Open Weather Map ***********/
#define OPEN_WEATHER_EP   "http://api.openweathermap.org/data/2.5/weather?"
#define OPEN_WEATHER_KEY  "&APPID=116950798518f4c877a6923dfeb65fe9"
#define CITY_ID_CHI       "id=4887398"   // city id for Chicago        
#define CITY_ID_STU       "id=2825297"   // city id for Stuttgart
/****************************************/

/********** Global State ****************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/*************** Feeds ******************/
// Setup a feed called 'onoffbutton' for subscribing to changes.
Adafruit_MQTT_Subscribe light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Subscribe noise = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/noise");
Adafruit_MQTT_Subscribe humidity = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Subscribe temp = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temp");
/****************************************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600); 
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

  mqtt.subscribe(&temp); 
  mqtt.subscribe(&noise); 
}

void loop() {
  MQTT_connect(); 

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(3100))) {
    if (subscription == &temp) {
      Serial.print(F("temp:"));
      Serial.print((char *)temp.lastread);    // SEND IT 
      delay(250); 
      getOpenWeatherMap(); 
    }
    else if (subscription == &noise) {
      Serial.print(F("noise:"));
      Serial.print((char *)noise.lastread); 
    }
  }
}

void getOpenWeatherMap() {
  char connectionStringCHI[128] = OPEN_WEATHER_EP;  
  char connectionStringSTU[128] = OPEN_WEATHER_EP; 

  strcat(connectionStringCHI, CITY_ID_CHI); 
  strcat(connectionStringCHI, OPEN_WEATHER_KEY);
  strcat(connectionStringSTU, CITY_ID_STU); 
  strcat(connectionStringSTU, OPEN_WEATHER_KEY);
  // get Chicago data
  sendWeatherRequest(connectionStringCHI, 0);      
  delay(250); 
  // get Stuttgart data
  sendWeatherRequest(connectionStringSTU, 1);     
}

void sendWeatherRequest(char *connectionString, int num) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(connectionString); 
    http.addHeader("Content-Type", "text/plain");
  
    int httpCode = http.POST("");
    String payload = http.getString();
    if (httpCode == 200) {
      int payloadSize = payload.length(); 
      char *payloadCp = (char *)malloc(payloadSize);
      payload.toCharArray(payloadCp, payloadSize);   
      char tempVal[7]; 
      char *tempLoc = strstr(payloadCp, "temp\":"); 
      if (tempLoc != NULL) {
        strncpy(tempVal, tempLoc+6, 6);
        tempVal[6] = 0; 
        Serial.print("temp-out");
        Serial.print(num);
        Serial.print(":"); 
        Serial.println(tempVal);
      }
    }
    else {
      Serial.println("Error! HTTP Response:"); 
      Serial.print(httpCode);
      Serial.print(": "); 
      Serial.println(payload); 
    }
    http.end(); 
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
