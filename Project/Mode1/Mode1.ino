#include <Wire.h>

// Initialization
const int firstJunctionLEDs[] = {13, 12, 11, 9, 10, 8};


const int firstJunctionWest[] = {13, 12, 11};
const int firstJunctionSouth[] = {8, 10, 9};

long unsigned int time_now;
long unsigned int timeZero;
long unsigned int period = 10000;
float dutyCycle = 0.5;
bool changeFirst = false;
bool isChanging = false;

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
int lightMin = 0;
int lightMax = 300;

// Cars related
int carW = 0;
int carS = 0;
bool carInW = false;
bool carInS = false;

void setup() {
  for (int i = 0; i < sizeof(firstJunctionLEDs) / sizeof(firstJunctionLEDs[0]); i++) {
    pinMode(firstJunctionLEDs[i], OUTPUT);
  }
  //initialState();
  pinMode(firstLightWest, INPUT);
  pinMode(firstLightSouth, INPUT);
  Serial.begin(9600);
  timeZero = millis();
}


void loop() {
  readCars();
  if ((millis() > timeZero + (period * dutyCycle) - 1000) && !isChanging) {
    handleYellow();
    isChanging = !isChanging;
  }
  if (millis() > timeZero + (period * (1 - dutyCycle)) && isChanging) {
    handleChange();
    timeZero = millis();
    isChanging = !isChanging;
    calculateDutyCycle();
    carW = 0;
    carS = 0;
  }
}

// TODO, initialize the RED and GREEN light according to the flag
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


void readCars(){
  firstLightWestValue = analogRead(firstLightWest);
  firstLightSouthValue = analogRead(firstLightSouth);
  firstLightWestMapped = map(firstLightWestValue, lightMin, lightMax, 0, 255);
  firstLightSouthMapped = map(firstLightSouthValue, lightMin, lightMax, 0, 255);

  if (firstLightWestMapped < 50 && !carInW) {
    carW ++;
    carInW = true;
  }
  else if (firstLightWestMapped >= lightMax/2)
    carInW = false;

  if (firstLightSouthMapped < 50 && !carInS) {
    carS ++;
    carInS = true;
  }
  else if (firstLightSouthMapped >= lightMax/2)
    carInS = false;
}

void calculateDutyCycle() {
  //  Duty_cycle_S = cars_S / (cars_S + cars_W)
  //  Duty_cycle_W = cars_W / (cars_S + cars_W)
  //  5/20 = 25% ≤ Duty cycle ≤ 15/20 = 75%
  if (carS == 0 && carW == 0)
    return;

  if (carS == 0) {
    dutyCycle = 0.75;
    return;
  }

  if (carW == 0) {
    dutyCycle = 0.25;
    return;
  }
  dutyCycle = carW / (carS + carW);
}
