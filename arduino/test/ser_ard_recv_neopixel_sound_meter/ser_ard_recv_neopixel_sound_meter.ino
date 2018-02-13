/*  neopixel  */
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/*  led matrix  */
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

/*  neopixel  */
#define PIN            6
#define NUMPIXELS      3

/**************** Other *****************/
#define INPUT_SIZE  31
#define TEMP_MAX    50          // max temp in celcius 
#define TEMP_MIN    -40        
/****************************************/

/*  led matrix  */
const int maxScale = 8;
const int redZone = 6;
const int yellowZone = 4;
const float maxVoltage = 3.3; 
const int maxVal = 676;     // 1024 for 5v, 676 for 3.3v

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

void setup() {
  Serial.begin(57600);
  pixels.begin(); 
  pixels.show();    // initialze pixels to off
  matrix.begin(0x70);  // pass in the address
  Serial.println("Starting..."); 
  delay(3000); 
}

void loop() {
  // update 
  
  // check for temp updates for neopixels   
  char incomingSerialData[INPUT_SIZE+1] = {0};
  if (getCommand(incomingSerialData) > 0) {
    parseCommand(incomingSerialData); 
  }
  delay(50); 
}

void updateSoundScroll(float reading) {
  //unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
  
  unsigned int signalMax = 0;
  unsigned int signalMin = maxVal;
  //float convert = reading * (float)maxVal / maxVoltage; 
//  while (millis() - startMillis < sampleWindow)
//  {
//    sample = (int)(reading * maxVoltage / maxVal); 
//    if (sample < maxVal)  // toss out spurious readings
//    {
//     if (sample > signalMax)
//     {
//      signalMax = sample;  // save just the max levels
//     }
//     else if (sample < signalMin)
//     {
//      signalMin = sample;  // save just the min levels
//     }
//    }
//   }
//   peakToPeak = signalMax - signalMin;
   Serial.print(reading); 
//   Serial.print("   "); 
//   Serial.print(signalMax);
//   Serial.print("   "); 
//   Serial.print(signalMin); 
//   Serial.print("   ");
//   Serial.print(peakToPeak);
//   Serial.print("   "); 
//   Serial.print(peakToPeak*3.3/(float)maxVal); 

   // map 1v p-p level to the max scale of the display
   //if (maxVal - peakToPeak <= 10) peakToPeak = maxVal;        // increase p2p if within 10 of maxVal
   int displayPeak = mapfloat2int(reading, 0.0, maxVoltage, 0, maxScale);
   Serial.print("   ");
   Serial.println(displayPeak); 
   delay(100);
    
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
   matrix.writeDisplay();  // write the changes we just made to the display
}

void RGB_Map(int index, float temp) {
  if (temp > 200) temp-=273.15; 
  int red = 0, green = 0, blue = 0; 
  float maximum = (float) TEMP_MAX; 
  float minimum = (float) TEMP_MIN; 
  float ratio = 2 * (temp - minimum) / (maximum - minimum);
  red = (int)(max(0, 255*(ratio-1))); 
  blue = (int)(max(0, 255*(1-ratio))); 
  green = 255 - blue - red; 
  // 
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
  //Serial.println(F("set command"));
  if (strcmp(feed, "temp") == 0) {
    float tempVal = atof(value); 
    RGB_Map(0, tempVal); 
  }
  else if (strcmp(feed, "temp-out0") == 0) {
    float tempVal = atof(value); 
    RGB_Map(1, tempVal);
  }
  else if (strcmp(feed, "temp-out1") == 0) {
    float tempVal = atof(value);
    RGB_Map(2, tempVal); 
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

