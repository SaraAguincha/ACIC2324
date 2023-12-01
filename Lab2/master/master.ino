#include <Wire.h>

int potentiometerSensor = 0;
int potentiometerMapped = 0;
int potentiometerPin = A3;


int lightSensor = 0;
int lightMapped = 0;
int lightPin = A1;

int lightMin = 0;
int lightMax = 1023;

int tempSensor = 0;
int tempMapped = 0;
int tempPin = A0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);           // start serial for output
  Wire.begin();
  pinMode(tempPin, INPUT);
  pinMode(lightPin, INPUT);
  pinMode(potentiometerPin, INPUT);
}


void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(4); // transmit to device #4
  
  // Potentiometer section
  Wire.write("P");        // sends five bytes
  potentiometerSensor = analogRead(potentiometerPin);
  potentiometerMapped = map(potentiometerSensor, 0, 1023, 0, 180);
  Wire.write(potentiometerMapped);// sends one byte 
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(4);

  Wire.write("L");
  lightSensor = analogRead(lightPin);
  lightMapped = map(lightSensor, lightMin, lightMax, 0, 255);
  Wire.write(lightMapped);// sends one byte 
  //Serial.println(lightSensor);
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(4);
  Wire.write("T");
  tempSensor = analogRead(tempPin);
  tempMapped = (((tempSensor / 1024.0) * 5.0 ) - 0.5 ) * 100;
  Wire.write(tempMapped);// sends one byte

  Wire.endTransmission();    // stop transmitting

  delay(1000);
}
