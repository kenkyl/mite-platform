const unsigned int sensorPin = 0; 
const unsigned int ledPin = 14; 

int lightLevel, high = 0, low = 1023; 

void setup() {
  Serial.begin(115200);
  delay(100); 

  pinMode(ledPin, OUTPUT); 

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100); 

  lightLevel = analogRead(sensorPin); 
  
  manualTune(); 

  Serial.println(lightLevel); 

  analogWrite(ledPin, lightLevel); 
}


void manualTune() {
  lightLevel = map(lightLevel, 0, 1023, 0, 255); 
  lightLevel = constrain(lightLevel, 0, 255); 
}


