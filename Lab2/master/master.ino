#include <Wire.h>
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
}

int sensorValue = 0;
int mappedValue = 0;

void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write("sensor value is ");        // sends five bytes
  sensorValue = analogRead(A3);
  mappedValue = map(sensorValue, 0, 1023, 1, 24);
  Wire.write(mappedValue);// sends one byte  
  Wire.endTransmission();    // stop transmitting

  delay(500);
}
