#include <Wire.h>

int delaySensor = 0;
int delayMapped = 0;
int delayPin = A2;
unsigned int delayTime = 0;
unsigned int delayRate = 200;

int blinkSensor = 0;
int blinkMapped = 0;
int blinkPin = A3;
unsigned int blinkTime = 0;
unsigned int blinkRate = 200;

int tempSensor = 0;
int tempMapped = 0;
int tempPin = A0;
unsigned int tempTime = 0;
unsigned int tempRate = 1000;

int lightSensor = 0;
int lightMapped = 0;
int lightPin = A1;
unsigned int lightTime = 0;
unsigned int lightRate = 200;

int lightMin = 1;
int lightMax = 900;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);           // start serial for output
  Wire.begin();
  pinMode(tempPin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(blinkPin, INPUT);
}


void loop() {
  // put your main code here, to run repeatedly:
  sendBlink();
  sendLight();
  sendTemp();
}

void sendBlink()
{
  if (millis() < blinkTime + blinkRate)
    return;
  blinkTime = millis();
  // blink section
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write("P");        // sends five bytes
  blinkSensor = analogRead(blinkPin);
  blinkMapped = map(blinkSensor, 0, 1023, 0, 180);
  Wire.write(blinkMapped);// sends one byte 
  Wire.endTransmission();    // stop transmitting
}

void sendLight()
{
  if (millis() < lightTime + lightRate)
    return;
  lightTime = millis();
  Wire.beginTransmission(4);
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
  Wire.write("T");
  tempSensor = analogRead(tempPin);
  tempMapped = (((tempSensor / 1024.0) * 5.0 ) - 0.5 ) * 100;
  Wire.write(tempMapped);// sends one byte

  Wire.endTransmission();    // stop transmitting
}
