#include <Wire.h>

// Initialization
const int firstJunctionLEDs[] = {13, 12, 11, 9, 10, 8};


const int firstJunctionWest[] = {13, 12, 11};
const int firstJunctionSouth[] = {8, 10, 9};

long unsigned int time_now;
long unsigned int timeZero;
long unsigned int period = 10000;
bool changeFirst = true;

// Section related with the light sensors
int firstLightWest = A0;
int firstLightSouth = A1;
int firstLightWestValue = 0;
int firstLightSouthValue = 0;
int firstLightWestMapped = 0;
int firstLightSouthMapped = 0;


// Minimum and maximum values for the light
// Value Max corresponds to the sensor value when close to a ceiling lamp
// Value Min corresponds to the sensor value when completely covered
int lightMin = 10;
int lightMax = 400; 

void setup() {
  for (int i = 0; i < sizeof(firstJunctionLEDs) / sizeof(firstJunctionLEDs[0]); i++) {
    pinMode(firstJunctionLEDs[i], OUTPUT);
    //digitalWrite(firstJunctionLEDs[i], HIGH);
  }
  initialState();
  pinMode(firstLightWest, INPUT);
  pinMode(firstLightSouth, INPUT);
  Serial.begin(9600);
}


void loop() {
  handleChange();
  timeZero = millis();
  while (millis() < timeZero + (period/2) - 1000 ) {}
  handleYellow();
  while (millis() < timeZero + (period/2)) {}
}

void initialState () {
  int counter = 0;
  while (counter < 3) {
    digitalWrite(10, HIGH);
    digitalWrite(12, HIGH);
    time_now = millis();
    while(millis() < time_now + 1000)
    {
      // empty
    }
    digitalWrite(10, LOW);
    digitalWrite(12, LOW);
    time_now = millis();
    while(millis() < time_now + 1000)
    {
      // empty
    }
    counter++;
  }
}

void handleYellow() {
  if (changeFirst) {
    digitalWrite(firstJunctionWest[0], LOW);
    digitalWrite(firstJunctionSouth[2], LOW);
  }
  else {
    digitalWrite(firstJunctionWest[2], LOW);
    digitalWrite(firstJunctionSouth[0], LOW);
  }
  digitalWrite(firstJunctionWest[1], HIGH);
  digitalWrite(firstJunctionSouth[1], HIGH);
  changeFirst = !changeFirst;
}

void handleChange(){
  // turns off the yellow lights in both junctions
  digitalWrite(firstJunctionWest[1], LOW);
  digitalWrite(firstJunctionSouth[1], LOW);

  // if true, west starts green and south red
  if (changeFirst) {
      digitalWrite(firstJunctionWest[0], HIGH);
      digitalWrite(firstJunctionSouth[2], HIGH);
  }
  else {
    digitalWrite(firstJunctionWest[2], HIGH);
    digitalWrite(firstJunctionSouth[0], HIGH);
  }
}
