// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      3

/**************** Other *****************/
#define INPUT_SIZE  31
#define TEMP_MAX    50          // max temp in celcius 
#define TEMP_MIN    -40        
/****************************************/

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

void setup() {
  Serial.begin(57600);
  pixels.begin(); 
  pixels.show();    // initialze pixels to off
  Serial.println("Starting..."); 
  delay(3000); 
}

void loop() {
  // put your main code here, to run repeatedly:
  char incomingSerialData[INPUT_SIZE+1] = {0};
  if (getCommand(incomingSerialData) > 0) {
    parseCommand(incomingSerialData); 
  }
  delay(50); 
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
  Serial.println(F("set command"));
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
}

