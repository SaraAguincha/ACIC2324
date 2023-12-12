#include <Wire.h>

// Sample rate for each of the sensors
unsigned int blinkRate = 200;
unsigned int tempRate = 2000;
unsigned int lightRate = 100;

// Section related with the delay sensor (Potentiometer)
// Related to question 8
int delaySensor = 0;
int delayMapped = 0;
int delayPin = A2;
unsigned int delayTime = 0;

// Section related with the blinking sensor (Potentiometer)
int blinkSensor = 0;
int blinkMapped = 0;
int blinkPin = A3;
unsigned int blinkTime = 0;

// Section related with the temperature sensor
int tempSensor = 0;
int tempMapped = 0;
int tempPin = A0;
unsigned int tempTime = 0;

// Section related with the light sensor
int lightSensor = 0;
int lightMapped = 0;
int lightPin = A1;
unsigned int lightTime = 0;

// Minimum and maximum values for the light
// Value Max corresponds to the sensor value when close to a ceiling lamp
// Value Min corresponds to the sensor value when completely covered
int lightMin = 1;
int lightMax = 900;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(tempPin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(blinkPin, INPUT);
  pinMode(delayPin, INPUT);
}


void loop() {
  sendBlink();
  sendLight();
  sendTemp();
  // Related to question number 8
  delayLoop();
}

// Variable delay of [0, 1000] ms 
void delayLoop()
{
  // delay section
  delaySensor = analogRead(delayPin);
  delayMapped = map(delaySensor, 0, 1023, 0, 1000);
  delayTime = millis();
  // Introduces a variable delay in the processor
  while(millis() < delayTime + delayMapped)
  {
    //empty on purpose
  }
}

void sendBlink()
{
  if (millis() < blinkTime + blinkRate)
    return;
  blinkTime = millis();
  // blink section
  Wire.beginTransmission(4);
  // Every blink message starts with "P"
  Wire.write("P");
  blinkSensor = analogRead(blinkPin);
  blinkMapped = map(blinkSensor, 0, 1023, 0, 180);
  Wire.write(blinkMapped);
  Wire.endTransmission();
}

void sendLight()
{
  if (millis() < lightTime + lightRate)
    return;
  lightTime = millis();
  Wire.beginTransmission(4);
  // Every light sensor message starts with "L"
  Wire.write("L");
  lightSensor = analogRead(lightPin);
  Serial.println(lightSensor);
  lightMapped = map(lightSensor, lightMin, lightMax, 0, 255);
  Wire.write(lightMapped);
  Wire.endTransmission();
}

void sendTemp()
{
  if (millis() < tempTime + tempRate)
    return;
  tempTime = millis();
  Wire.beginTransmission(4);
  // Every temperature message starts with "T"
  Wire.write("T");
  tempSensor = analogRead(tempPin);
  tempMapped = (((tempSensor / 1024.0) * 5.0 ) - 0.5 ) * 100;
  Wire.write(tempMapped);

  Wire.endTransmission();
}
