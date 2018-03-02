#include <stdio.h>
#include <string.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266WiFi.h>

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

/**************** Other *****************/
#define INPUT_SIZE 31
#define PIN_LED    4

/****************************************/

/********** Global State ****************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY); 
/****************************************/

/*************** Feeds ******************/
// Setup pub/sub feeds 
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish light = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish noise = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/noise");
Adafruit_MQTT_Subscribe switch_led = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/switch_led");
Adafruit_MQTT_Subscribe switch_buz = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/switch_buzzer");
/****************************************/

void setup() {
  pinMode(PIN_LED, OUTPUT);

  // define pin modes for tx, rx:
  Serial.begin(57600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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

  Serial.println("\nSoftware serial RECV test started");
  
  mqtt.subscribe(&switch_led); 

  // wait 3 seconds to allow arduino to start transmitting data
  delay(3000); 
}

unsigned int led_timer = 0; 
void loop() {
  MQTT_connect(); 
  //delay(50); 
  // reset led if on for greater than 5 seconds 
  if (led_timer >= 5000) {
    digitalWrite(PIN_LED, LOW); 
    led_timer = 0; 
    Serial.println("LED OFF"); 
  }

  // check for push of led button 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &switch_led) {
      Serial.println("LED ON"); 
      digitalWrite(PIN_LED, HIGH); 
      led_timer = 0; 
    }
  }
  // increment led timer by 50 ms
  led_timer += 100; 

  //
  char incomingSerialData[INPUT_SIZE+1] = {0};
  if (getCommand(incomingSerialData) > 0) {
    parseCommand(incomingSerialData); 
  }

}

int getCommand(char *incomingSerialData)
{
  int incomingSerialDataIndex = 0; 

  while(Serial.available() > 0)
  {
    incomingSerialData[incomingSerialDataIndex] = Serial.read();
    if (incomingSerialData[incomingSerialDataIndex] != 13) {
      incomingSerialDataIndex++;
    }
    // set last position to null terminating char
    incomingSerialData[incomingSerialDataIndex] = 0;
  }
  
  return incomingSerialDataIndex; 
}

void parseCommand(char *commandString) {
  // split the commandString into two segments separated by colon
  char *command = strtok(commandString, ":"); 
  if (command == NULL) return;    // exit function if no colon found
  char pubSet[2][INPUT_SIZE/2+1]; 
  int index = 0;  
  // grab the feed and value from the command string and store separately in pubSet
  while (command != NULL && index < 2) { 
    if (command != NULL && command != 0 && strlen(command)>3) {
      strcpy(pubSet[index], command); 
      index++;
    }
    command = strtok(NULL, ":"); 
  }  
  if (index > 1) {
    if (pubSet[0] != NULL && pubSet[1] != NULL) {
      Serial.print("pubSet[0]= ");
      Serial.print(pubSet[0]); Serial.print(" ");
      Serial.print("pubSet[1]= ");
      Serial.println(pubSet[1]);
      publishToFeed(pubSet[0], pubSet[1]); 
    }
  }
}

void publishToFeed(char *feed, char *value) {
  Serial.print("feed= ");
  Serial.print(feed); Serial.print(" ");
  Serial.print("value= ");
  Serial.println(value);
  if (strcmp(feed, "noise") == 0) {
    if (! noise.publish(value)) {
      Serial.println(F("noise publish failed"));
    } else {
      Serial.println(F("noise publish OK!"));
    }
  }
  else if (strcmp(feed, "humidity") == 0) {
    if (! humidity.publish(value)) {
      Serial.println(F("humidity publish failed"));
    } else {
      Serial.println(F("humidity publish OK!"));
    }
  }
  else if (strcmp(feed, "temp") == 0) {
    if (! temp.publish(value)) {
      Serial.println(F("temp publish failed"));
    } else {
      Serial.println(F("temp publish OK!"));
    }
  }
  else if (strcmp(feed, "light") == 0) {
    if (! light.publish(value)) {
      Serial.println(F("light publish failed"));
    } else {
      Serial.println(F("light publish OK!"));
    }
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

