/*
  Software serial send multiple sensor data 
  to ESP8266 via software serial

 */
 
/* includes */
// general
#include <stdio.h> 
// tsl
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
// dht
#include "DHT.h"

/* defines */
#define PIN_DHT 4
#define TYPE_DHT DHT22
#define PIN_RX 10
#define PIN_TX 11 
#define OUTPUT_SIZE 31

/* globals */
// tsl
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
// dht
DHT dht(PIN_DHT, TYPE_DHT);
// max
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() {  
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("\n***** ENVIRONMENTAL SENSOR DATA COLLECTOR STARTED *****");

  /* Initialise the sensors */
  // (1) tsl
  // use tsl.begin() to default to Wire, 
  // tsl.begin(&Wire2) directs api to use Wire2, etc.
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  /* Display some basic information on this sensor */
  displaySensorDetails();
  /* Setup the sensor gain and integration time */
  configureSensor();
  /* We're ready to go! */
  Serial.println("(1) TSL sensor intialized");

  // (2) dht 
  dht.begin();
  Serial.println("(2) DHT sensor intialized");
  delay(100); 

  // (3) max 
  Serial.println("(3) MAX sensor intialized");
  delay(100); 
}

unsigned int timer = 0; 
void loop() { 

    if (timer >= 60000) timer = 0;  // reset timer each minute
    // semd all sensor data every 30 seconds 
    if (timer % 60000 == 0) { 
      sendMAX(); 
      delay(50); 
      sendTSL(); 
      delay(50);
      sendDHT(); 
    }
    // send light and noise every 2 seconds 
    else if (timer % 6000 == 0) {
      sendMAX();
      delay(50);    
      sendTSL();
    }
    else if (timer % 3000 == 0) { 
      sendTSL(); 
    }
    else if (timer % 2000 == 0) {
      sendMAX(); 
    }
    
    delay(1000); 
    timer+=1000; 
}

/***** SEND FUNCTIONS *****/
void sendTSL() {
  float valTSL; 
  if (readTSL(&valTSL) == true) {
    Serial.print("light:"); 
    Serial.println(valTSL); 
  } else {
    Serial.println("MAX read fail!");
  }
}
void sendMAX() {
  double valMAX; 
  if (readMAX(&valMAX) == true) {
    Serial.print("noise:"); 
    Serial.println(valMAX); 
  } else {
    Serial.println("MAX read fail!");
  }
}
void sendDHT() {
  float valDHT[2]; 
  if (readDHT(valDHT) == true) {
    Serial.print("humidity:"); 
    Serial.println(valDHT[0]); 
    delay(50); 
    Serial.print("temp:"); 
    Serial.println(valDHT[1]);
  } else {
    Serial.println("DHT read fail!");
  }
}

/***** HELPER FUNCTIONS *****/
// tsl
/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  //tsl.setGain(TSL2561_GAIN_8X);
  //tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("16x");
  Serial.print  ("Timing:       "); Serial.println("101 ms");

  Serial.println("------------------------------------");
}

boolean readTSL(float *valTSL) {
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    *valTSL = event.light; 
    return true; 
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
    return false; 
  }
}

// dht 
boolean readDHT(float *valDHT) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  // set values and return 
  valDHT[0] = h; 
  valDHT[1] = t; 
  return true; 
}

// max 
boolean readMAX(double *valMAX) {
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   double voltageInput = 3.3;       
   unsigned int inputMax = 676;   // 1024 for 5V input
   unsigned int signalMax = 0;
   unsigned int signalMin = inputMax;     


   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < inputMax)  // toss out spurious readings
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
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * voltageInput) / inputMax;  // convert to volts
   if (volts >= 0.0 && volts <= voltageInput) {
      *valMAX = volts;
      return true; 
   }
}
