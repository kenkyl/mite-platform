/****************************************
Scrolling Sound Meter Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// Include the Matrix code for display
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

const int maxScale = 8;
const int redZone = 6;
const int yellowZone = 4;
const int maxVal = 676;     // 1024 for 5v

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() 
{
   Serial.begin(57600);

   matrix.begin(0x70);  // pass in the address
}


void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;
   unsigned int signalMin = maxVal;

   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0); 
      if (sample < maxVal)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;
   Serial.print(signalMax);
   Serial.print("   "); 
   Serial.print(signalMin); 
   Serial.print("   ");
   Serial.print(peakToPeak);
   Serial.print("   "); 
   Serial.print(peakToPeak*3.3/(float)maxVal); 

   // map 1v p-p level to the max scale of the display
   if (maxVal - peakToPeak <= 10) peakToPeak = maxVal;        // increase p2p if within 10 of maxVal
   int displayPeak = map(peakToPeak, 0, maxVal, 0, maxScale);
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
