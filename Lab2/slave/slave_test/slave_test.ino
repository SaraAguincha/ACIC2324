#include <Wire.h>
int groups = 32;
int bytesTest = 32;
int counter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);           // start serial for output
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
}

void loop() {
  // put your main code here, to run repeatedly:
  if (counter == bytesTest * groups)
  {
    Wire.beginTransmission(8);
    Wire.write("K", 1);
    Wire.endTransmission();
    counter++;
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  char c = ' ';
  while(0 < Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // receive byte as a character
    counter ++;
  }
}