/*
 * File:  collect_esp_recv
 * Desc:  Code for the ESP8266 Huzzah in the Collection module (1b). Receives sensor data
 *        from the corresponding Arduino and forwards it via WiFi to the MQTT broker.
 *        Also subscribes to button events and turns on buzzer LED when the corresponding
 *        command is received over MQTT. Pitches.h file include for buzzer sounds. 
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
#include "pitches.h"
/************ ESP8266 Wifi **************/
#include <ESP8266WiFi.h>
/**************** MQTT ******************/
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/*********** WIFI Credentials ***********/
#define WLAN_SSID       "connectory 2"    // UPDATE WIFI NETWORK HERE IF NEEDED 
#define WLAN_PASS       "c0nn3ct0ry2"     // UPDATE WIFI PASSWORD HERE IF NEEDED 

/********** Adafruit.io Setup  **********/
// NOTE: Would replace with another MQTT Broker here!
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883               // use 8883 for SSL
#define AIO_USERNAME    "connectory_io"
#define AIO_KEY         "76805a2a35c84583a9ac77cd9ec5f546"

/**************** Other *****************/
#define INPUT_SIZE  31       // num bytes for serial messages
#define PIN_LED     12       // pin for LED
#define PIN_BUZ     14       // pin for buzzer

/********** Global State ****************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
unsigned int led_timer = 0, buz_timer = 0; 
bool buz_on = false;  

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

// Run once upon startup of the ESP
void setup() {
  Serial.begin(57600);    // NOTE: Serial rate can be changed but must match collect_ard_send
  // set output pins for buzzer and LED
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZ, OUTPUT);

  // connect to Wi-Fi 
  Serial.println();
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("-------------------------------------");

  // subscribe to necessary feeds 
  mqtt.subscribe(&switch_led); 
  mqtt.subscribe(&switch_buz); 

  // wait 3 seconds to allow arduino to start transmitting data
  delay(3000);  
  Serial.println("collect_esp_recv initiated...");
}

// Loop constantly while the ESP is on
void loop() {
  MQTT_connect(); // verify MQTT connection
  
  // reset led if on for greater than 5 seconds 
  if (led_timer >= 5000) {
    digitalWrite(PIN_LED, LOW); 
    led_timer = 0; 
  }
  // set buzzer tone based on timing and turn off after 6 seconds 
  if (buz_timer >= 6000) {
    noTone(PIN_BUZ); 
    buz_on = false; 
    buz_timer = 0; 
  }
  else if (buz_on == true && buz_timer >= 4000) {
    tone(PIN_BUZ, NOTE_G3, 2000);
  }
  else if (buz_on == true && buz_timer >= 2000) {
    tone(PIN_BUZ, NOTE_G4, 2000); 
  }
  
  // check for push of LED or Buzzer buttons 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(50))) {
    if (subscription == &switch_led) {
      Serial.println("LED ON"); 
      digitalWrite(PIN_LED, HIGH); 
      led_timer = 0; 
    }
    else if (subscription == &switch_buz) {
      Serial.println("BUZZER ON");
      tone(PIN_BUZ, NOTE_G2, 2000); 
      buz_on = true; 
      buz_timer = 0;
    }
  }
  // increment led timer by 50 ms
  led_timer += 50; 
  buz_timer += 50;

  // check for incoming sensor data from arduino -> parse and send to cloud if found
  char incomingSerialData[INPUT_SIZE+1] = {0};
  if (getCommand(incomingSerialData) > 0) {
    parseCommand(incomingSerialData); 
  }
}

/*
 * Name:  getCommand 
 * Args:  char *incomingSerialData
 * Desc:  Checks the Serial Rx line for incoming data and, if found, writes it into the  
 *        buffer <incomingSerialData> that is passed by reference (as a pointer)
 * Rets:  the number of characters read as an int
 */
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

/*
 * Name:  parseCommand
 * Args:  char *commandString
 * Desc:  Accepts the pointer <commandString> and splits its contents at the first colon
 *        found. Expects input in the form "<feed>:<value>". Returns early if no colon is
 *        found. If a valid command pair is identified, it is passed to setCommand().
 * Rets:  void
 */
void parseCommand(char *commandString) {
  // split the commandString into two segments separated by colon
  char *command = strtok(commandString, ":"); 
  if (command == NULL) return;    // exit function if no colon found
  char commandPair[2][INPUT_SIZE/2+1]; 
  int index = 0;  
  // grab the feed and value from the command string and store separately in commandPair
  while (command != NULL && index < 2) { 
    if (command != NULL && command != 0 && strlen(command)>3) {
      strcpy(commandPair[index], command); 
      index++;
    }
    command = strtok(NULL, ":"); 
  }  
  if (index > 1) {
    if (commandPair[0] != NULL && commandPair[1] != NULL) {
      // optional print statements to verify proper parsing 
      //Serial.print("commandPair[0]= ");
      //Serial.print(commandPair[0]); Serial.print(" ");
      //Serial.print("commandPair[1]= ");
      //Serial.println(commandPair[1]);
      // pass the parsed command pair to to getCommand to act on it 
      publishToFeed(commandPair[0], commandPair[1]); 
    }
  }
}

/*
 * Name:  publishToFeed    
 * Args:  char *feed, char *value
 * Desc:  Checks an incoming <feed>, <value> pair and publishes to corresponding MQTT feed
 * Rets:  void
 */
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
  // if the feed does not match any specified, the command is ignored 
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

