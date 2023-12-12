#include <Wire.h>
int redPin = 4;
int bluePin = 6;
int yellowPin = 5;

unsigned long time_now = 0;
int mappedFrequency = 0;

// Value of temperature that triggers the red LED actuator
int temperatureTrigger = 18;

void setup() {
  Serial.begin(9600);
  // join i2c bus with address #4
  Wire.begin(4);
  Wire.onReceive(receiveEvent);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
}

void loop() {
  blinkLoop();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  char c = ' ';
  // loop through all but the last
  while(1 < Wire.available()) 
  {
    // receive byte as a character
    c = Wire.read(); 
  }
  // receive last byte as an integer
  int x = Wire.read();    

  // Round robin with interrupts
  switch (c)
  {
    // Every temperature message starts with "T"
    case 'T':
      tempActuator(x);
      break;
    // Every light sensor message starts with "L"
    case 'L':
      lightActuator(x);
      break;
    case 'P':
    // Every blink message starts with "P"
      blinkActuator(x);
      break;
  }
}

void tempActuator(int temp)
{
  if (temp > temperatureTrigger)
    digitalWrite(redPin, HIGH);
  else
    digitalWrite(redPin, LOW);
}

// Updates the frequency in which the actuator should blink
void blinkActuator(int frequency)
{
  mappedFrequency = map(frequency, 0, 180, 200, 2000);
}

// Loop that makes the actuator blink
void blinkLoop()
{
  time_now = millis();
  digitalWrite(bluePin, HIGH);
  while(millis() < time_now + mappedFrequency)
  {
    // empty on purpose
  }
  digitalWrite(bluePin, LOW);
  time_now = millis();
  while(millis() < time_now + mappedFrequency)
  {
    // empty on purpose
  }
}

void lightActuator(int intensity)
{
  // Inversely proportional to the the value of the sensor
  analogWrite(yellowPin, 255 - intensity);
}
