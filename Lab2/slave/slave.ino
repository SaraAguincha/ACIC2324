#include <Wire.h>
int redPin = 4;
int bluePin = 6;
int yellowPin = 5;

unsigned long time_now = 0;
int mappedFrequency = 0;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  blinkLoop();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  char c = ' ';
  while(1 < Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // receive byte as a character
    //Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);         // print the integer

  switch (c){
    case 'T':
      //Serial.print("Temperature is ");
      //Serial.println(x);
      tempActuator(x);
      break;
    case 'L':
      //Serial.print("Light is ");
      //Serial.println(x);
      lightActuator(x);
      break;
    case 'P':
      //Serial.print("blink is ");
      //Serial.println(x);
      blinkActuator(x);
      break;
  }
}

void tempActuator(int temp)
{
  if (temp > 10)
    digitalWrite(redPin, HIGH);
  else
    digitalWrite(redPin, LOW);
}

void blinkActuator(int frequency)
{
  mappedFrequency = map(frequency, 0, 180, 200, 2000);
}

void blinkLoop()
{
  time_now = millis();

  //Serial.println(time_now);

  digitalWrite(bluePin, HIGH);
  while(millis() < time_now + mappedFrequency){
  }
  digitalWrite(bluePin, LOW);
  time_now = millis();
  while(millis() < time_now + mappedFrequency){
  }
}

void lightActuator(int intensity)
{
  analogWrite(yellowPin, 255 - intensity);
}
