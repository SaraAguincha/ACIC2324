#include <Wire.h>
int groups = 32;
int bytesTest = 32;
unsigned long timeBefore;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);           // start serial for output
  Wire.begin(8);
  Wire.onReceive(receiveEvent); // register event

  timeBefore = micros();

  for(int j = 0; j < groups; j++)
  {
    Wire.beginTransmission(4);
    for(int i = 0; i < bytesTest; i++)
    {
      Wire.write("A" , 1);
    }
    Wire.endTransmission();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

void receiveEvent(int howMany)
{
  char c = ' ';
  while(0 < Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // receive byte as a character
  }
  unsigned long timeAfter = micros();

  unsigned long deltaTime = timeAfter - timeBefore;

  Serial.println("Took this much time: ");
  Serial.print(deltaTime);
}