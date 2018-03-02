/*
 * File:  display_ard_recv
 * Desc:  Code for the Arduino Uno in the Display module. Receives data over serial from 
 *        the corresponding ESP8266 and controls and 8x8 LED Matrix and 3 Neopixel LEDs
 *        to track relative noise and temperature levels accordingly. 
 *        
 * Created:   15 Feburary 2018
 * Modified:  02 March 2018
 * 
 * KEK1AD 
 * RBNA - CI/IO
 * Created for the MITE (Mini IoT Experience) Platform Demo in the Chicago Connectory 
 */

/************** Neopixel  ***************/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
/************* LED Matrix  **************/
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

/************** Neopixel  ***************/
#define PIN            6
#define NUMPIXELS      3

/**************** Other *****************/
#define INPUT_SIZE  31          // num bytes for serial messages 
#define TEMP_MAX    40          // max temp in celcius 
#define TEMP_MIN    -30         // min temp in celcius 

/************* LED Matrix  **************/
const int maxScale = 8;
const int redZone = 6;
const int yellowZone = 4;
const float maxReading = 100.0; 
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Run once upon startup of the Arduino
void setup() {
  Serial.begin(57600);
  pixels.begin(); 
  pixels.show();       // initialze pixels to off
  matrix.begin(0x70);  // pass in the address
  Serial.println("Starting..."); 
  delay(3000); 
}

// Loop constantly while the Arduino is on
void loop() {  
  // check for updates from ESP over serial    
  char incomingSerialData[INPUT_SIZE+1] = {0};  // buffer for incoming messages 
  if (getCommand(incomingSerialData) > 0) {
    parseCommand(incomingSerialData); 
  }
  delay(50); 
}

/*
 * Name:  updateSoundScroll(float reading) 
 * Desc:  Receive a sound level reading from the ESP and update one column of the LED matrix 
 *        accoriding to the level, where 0 squares illuminated is the min sound and 8 
 *        is the max. 
 */
void updateSoundScroll(float reading) {  
   if (maxReading - reading <= 5) reading = maxReading;  // increase reading if within 5 of max (100%) 
   int displayPeak = mapfloat2int(reading, 0.0, 100.0, 0, maxScale);  // map reading from percentage 0-8
    
   // Update the display:
   for (int i = 0; i < 7; i++)  // shift the display left
   {
      matrix.displaybuffer[i] = matrix.displaybuffer[i+1];
   }

   // draw the new sample
   for (int i = 0; i <= maxScale; i++)
   {
      if (i >= displayPeak)  // blank these pixels
      {
         matrix.drawPixel(i, 7, 0);
      }
      else if (i < yellowZone) 
      {
        matrix.drawPixel(i, 7, LED_GREEN);
      }
      else if (i < redZone) // draw in green
      {
         matrix.drawPixel(i, 7, LED_YELLOW);
      }
      else // Red Alert!  Red Alert!
      {
         matrix.drawPixel(i, 7, LED_RED);
      }
   }
   matrix.writeDisplay();  // write the changes to the display
}

/*
 * Name:  updateTempLED(int index, float temp) 
 * Desc:  Update the Neopixel LED at <index> to an RGB value corresponding to <temp>. The
 *        color is 
 */
// map rgb color to neopixels 
void updateTempLED(int index, float temp) {
  if (temp > 200) temp-=273.15; 
  int red = 0, green = 0, blue = 0; 
  float maximum = (float) TEMP_MAX; 
  float minimum = (float) TEMP_MIN; 
  float ratio = 2 * (temp - minimum) / (maximum - minimum);
  red = (int)(max(0, 255*(ratio-1))); 
  blue = (int)(max(0, 255*(1-ratio))); 
  green = 255 - blue - red; 
  // set pixel color based on calculation and display 
  pixels.setPixelColor(index, pixels.Color(red, green, blue)); 
  pixels.show(); 
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
      setCommand(pubSet[0], pubSet[1]); 
    }
  }
}

void setCommand(char *feed, char *value) {
  if (strcmp(feed, "temp") == 0) {
    float tempVal = atof(value); 
    updateTempLED(0, tempVal); 
  }
  else if (strcmp(feed, "temp-out0") == 0) {
    float tempVal = atof(value); 
    updateTempLED(1, tempVal);
  }
  else if (strcmp(feed, "temp-out1") == 0) {
    float tempVal = atof(value);
    updateTempLED(2, tempVal); 
  }
  else if (strcmp(feed, "noise") == 0) {
    float noiseVal = atof(value); 
    Serial.print(value);
    Serial.print("  ");
    Serial.println(noiseVal); 
    updateSoundScroll(noiseVal); 
  }
}

int mapfloat2int(float x, float in_min, float in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

