#include <Wire.h>

// Initialization
const int junctionLEDs[2][2][3] = {{{13, 12, 11}, {8, 10, 9}}, {{7, 6, 5}, {2, 4, 3}}};

// Time stuff
long unsigned int time_now[] = {0, 0};
long unsigned int timeZero[] = {0, 0};
long unsigned int period = 10000;
float dutyCycle[] = {0.5, 0.5};
bool changeFirst[] = {true, true};
bool isChanging[] = {false, false};

// Section related with the light sensors
const int light[2][2] = {{A0, A1}, {A2, A3}};
int lightValue[2][2] = {{0, 0},{0, 0}};
int lightMapped[2][2] = {{0, 0}, {0, 0}};

// Cars related
// First is West, then South
int cars[2][2] = {{0, 0}, {0, 0}};
bool carInJunction[2][2] = {{false, false}, {false, false}};


void setup() {
  for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      for (int k = 0; k < 3; k++) 
      {
        pinMode(junctionLEDs[i][j][k], OUTPUT);
      }
    }
  }
  for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      pinMode(light[i][j], INPUT);
    }
  }

  //initialState();
  Serial.begin(9600);
  timeZero[0] = millis();
}


void loop() 
{
  readCars();
  //Serial.println(dutyCycle[0]);
  //dutyCycle[0] = 0.25;
  for (int i = 0; i < 2; i++) 
  {
    if ((millis() > timeZero[i] + (period * dutyCycle[i]) - 1000)) 
    {
      if (millis() > timeZero[i] + (period * dutyCycle[i]))
      {
        if (millis() > timeZero[i] + period - 1000)
        {
          if (millis() > timeZero[i] + period)
          {
            handleChange(i);
            timeZero[i] = millis();
            
            Serial.print("Cars in junction ");
            Serial.print(i);
            Serial.print(": (West) ");
            Serial.print(cars[i][0]);
            Serial.print(", (South) ");
            Serial.println(cars[i][1]);
            Serial.println("\n");

            calculateDutyCycle(i);
            
            Serial.print("Duty cycle: ");
            Serial.println(dutyCycle[i]);
            
            cars[i][0] = 0;
            cars[i][1] = 0;
            isChanging[i] = false;
          }
          else if (!isChanging[i])
          {
            isChanging[i] = !isChanging[i];
            handleYellow(i);
          }
        }
        else if (isChanging[i])
        {
          isChanging[i] = !isChanging[i];
          handleChange(i);
        }
      }
      else if (!isChanging[i])
      {
        isChanging[i] = !isChanging[i];
        handleYellow(i);
      }
    }
  }
}

// TODO, initialize the RED and GREEN light according to the flag
void initialState () {
  int counter = 0;
  //const int junctionLEDs[2][2][3] = {{{13, 12, 11}, {8, 10, 9}}, {{7, 6, 5}, {2, 4, 3}}};

  while (counter < 3) {
    digitalWrite(10, HIGH);
    digitalWrite(12, HIGH);
    time_now[0] = millis();
    while(millis() < time_now[0] + 1000)
    {
      // empty
    }
    digitalWrite(10, LOW);
    digitalWrite(12, LOW);
    time_now[0] = millis();
    while(millis() < time_now[0] + 1000)
    {
      // empty
    }
    counter++;
  }
}

void handleYellow(int junction) {
  if (changeFirst[junction]) {
    digitalWrite(junctionLEDs[junction][0][0], LOW);
    digitalWrite(junctionLEDs[junction][1][2], LOW);
    
  }
  else {
    digitalWrite(junctionLEDs[junction][0][2], LOW);
    digitalWrite(junctionLEDs[junction][1][0], LOW);
  }
  digitalWrite(junctionLEDs[junction][0][1], HIGH);
  digitalWrite(junctionLEDs[junction][1][1], HIGH);
  changeFirst[junction] = !changeFirst[junction];
}

void handleChange(int junction){
  // turns off the yellow lights in both junctions
  digitalWrite(junctionLEDs[junction][0][1], LOW);
  digitalWrite(junctionLEDs[junction][1][1], LOW);

  // if true, west starts green and south red
  if (changeFirst[junction]) {
      digitalWrite(junctionLEDs[junction][0][0], HIGH);
      digitalWrite(junctionLEDs[junction][1][2], HIGH);
  }
  else {
    digitalWrite(junctionLEDs[junction][0][2], HIGH);
    digitalWrite(junctionLEDs[junction][1][0], HIGH);
  }
}


void readCars(){
  for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      lightValue[i][j] = analogRead(light[i][j]);
    }
  }

  // TODO add the calibration to initialState
  lightMapped[0][0] = map(lightValue[0][0], 45, 300, 0, 255);
  lightMapped[0][1] = map(lightValue[0][1], 20, 200, 0, 255);
  lightMapped[1][0] = map(lightValue[1][0], 45, 300, 0, 255);
  lightMapped[1][1] = map(lightValue[1][1], 45, 300, 0, 255);

  for (int i = 0; i < 2; i++) 
  {
    if (lightMapped[i][0] < 50 && !carInJunction[i][0]) 
    {
      cars[i][0]++;
      carInJunction[i][0] = true;
    }
    else if (lightMapped[i][0] >= 200/2)
      carInJunction[i][0] = false;

    if (lightMapped[i][1] < 50 && !carInJunction[i][1]) 
    {
      cars[i][1]++;
      carInJunction[i][1] = true;
    }
    else if (lightMapped[i][1] >= 300/3)
      carInJunction[i][1] = false;
  }
}

void calculateDutyCycle(int junction) {
  //  Duty_cycle_S = cars_S / (cars_S + cars_W)
  //  Duty_cycle_W = cars_W / (cars_S + cars_W)
  //  5/20 = 25% ≤ Duty cycle ≤ 15/20 = 75%
  if (cars[junction][0] == 0 && cars[junction][1] == 0)
  { 
    dutyCycle[junction] = 0.50;
    return;
  }

  dutyCycle[junction] = (float) cars[junction][1] / ( (float) cars[junction][1] + (float) cars[junction][0]);

  if (dutyCycle[junction] > 0.75)
  {
    dutyCycle[junction] = 0.75;
  }

  if (dutyCycle[junction] < 0.25)
  {
    dutyCycle[junction] = 0.25;
  }
}