/*
 * File:  display_esp_semd
 * Desc:  Code for the ESP8266 Huzzah in the Display module (2b). Connects to the MQTT 
 *        broker via Wifi and collects and forwards data to the corresponding Arduino 
 *        over serial. Also contains two buttons that when pressed publish to MQTT and 
 *        trigger events at the collection module (turning on either a buzzer or LED).
 *        
 * 1. [a.collect_ard_send]-->[b.colect_esp_recv]-->((cloud))
 * 
 * 2. [a.display_ard_recv]<--[b.display_esp_send]<--((cloud))
 * 
 * Created:   15 Feburary 2018
 * Modified:  12 March 2018
 * 
 * KEK1AD - kyle.kennedy@us.bosch.com
 * RBNA - CI/IO
 * Created for the MITE (Mini IoT Experience) Platform Demo in the Chicago Connectory 
 */

/*************** General ****************/
#include <stdio.h>
#include <string.h>
/************ ESP8266 Wifi **************/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
/**************** MQTT ******************/
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/*********** WIFI Credentials ***********/
#define WLAN_SSID   "connectory 2"        // UPDATE WIFI NETWORK HERE IF NEEDED 
#define WLAN_PASS   "c0nn3ct0ry2"         // UPDATE WIFI PASSWORD HERE IF NEEDED 

/********** Adafruit.io Setup  **********/
// NOTE: Would replace with another MQTT Broker here! 
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883               // use 8883 for SSL
#define AIO_USERNAME    "connectory_io"
#define AIO_KEY         "76805a2a35c84583a9ac77cd9ec5f546"

/*********** Open Weather Map ***********/
#define OPEN_WEATHER_EP   "http://api.openweathermap.org/data/2.5/weather?"
#define OPEN_WEATHER_KEY  "&APPID=116950798518f4c877a6923dfeb65fe9"
#define CITY_ID_CHI       "id=4887398"   // city id for Chicago        
#define CITY_ID_STU       "id=2825297"   // city id for Stuttgart

/**************** Other *****************/
#define PIN_SWITCH_LED     4
#define PIN_SWITCH_BUZ     5

/********** Global State ****************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/*************** Feeds ******************/
// Setup pub/sub feeds 
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe noise = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/noise");
Adafruit_MQTT_Subscribe temp = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish switch_led = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/switch_led");
Adafruit_MQTT_Publish switch_buz = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/switch_buzzer");

// Run once upon startup of the ESP
void setup() {
  Serial.begin(57600);  // NOTE: Serial rate can be changed but must match display_ard_recv
  // set input pins for buzzer and LED trigger buttons
  pinMode(PIN_SWITCH_LED, INPUT); 
  pinMode(PIN_SWITCH_BUZ, INPUT); 
  delay(100); 

  // connect to Wi-Fi 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // print Wifi details
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address= ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask= ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway= ");
  Serial.println(WiFi.gatewayIP());

  // subscribe to necessary feeds 
  mqtt.subscribe(&temp); 
  mqtt.subscribe(&noise); 

  Serial.println("display_esp_send initiated..."); 
}

// Loop constantly while the ESP is on
void loop() {
  MQTT_connect(); // verify MQTT connection

  // check for press of LED trigger button
  if (digitalRead(PIN_SWITCH_LED) == 1) {
    if (! switch_led.publish(1)) {
      Serial.println(F("switch_led publish failed"));
    } else {
      Serial.println(F("switch_led publish OK!"));
    }
  }
  // check for press of buzzer trigger button
  if (digitalRead(PIN_SWITCH_BUZ) == 1) {
    if (! switch_buz.publish(1)) {
      Serial.println(F("switch_buzzer publish failed"));
    } else {
      Serial.println(F("switch_buzzer publish OK!"));
    }
  }

  // check specified subscription(s) for updates 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    // if receiving temperature --> send to Arduino over serial and get weather data
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

/*
 * Name:  getOpenWeatherMap 
 * Args:  none 
 * Desc:  Creates HTTP request strings for OpenWeatherMap.org API and passes them to 
 *        sendWeatherRequest(char *connectionString, int num) function
 * Rets:  void
 */
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

/*
 * Name:  sendWeatherRequest 
 * Args:  char *connectionString, int num 
 * Desc:  Sends the HTTP request <connectionString> to the specified endpoint
 *        (OpenWeatherMap.org) and parses json response to get the temperature. <num> is 
 *        used to distinguish between different locations. 
 * Rets:  void
 */
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

/*
 * Name:  MQTT_connect 
 * Args:  none
 * Desc:  Checks connection to the MQTT broker (server) and attemps to connect or reconnect
 *        if not connected. Should be called in loop prior to communication attempts(). 
 * Rets:  void
 */
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
