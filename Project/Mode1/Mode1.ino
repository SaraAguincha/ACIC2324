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
int lightMin[2][2] = {{1023, 1023}, {1023, 1023}};
int lightMax[2][2] = {{0, 0}, {0, 0}};

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
  Serial.begin(9600);
  initialState();
  timeZero[0] = millis();
  timeZero[1] = millis();
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
  long unsigned int init_time = 0;
  //const int junctionLEDs[2][2][3] = {{{13, 12, 11}, {8, 10, 9}}, {{7, 6, 5}, {2, 4, 3}}};
  bool blink = true;
  init_time = millis();
  while (counter < 6) {

    lightCalibration();

    if(millis() > init_time + 1000)
    {
      // Yellow lights
      for (int i = 0; i < 2; i++)
      {
        digitalWrite(junctionLEDs[i][0][1], blink ? HIGH : LOW);
        digitalWrite(junctionLEDs[i][1][1], blink ? HIGH : LOW);
      }
      counter++;
      init_time = millis();
      blink = !blink;
    }
  }


  for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      Serial.print("In junction ");
      Serial.print(i);
      Serial.print(" MinLight: ");
      Serial.print(lightMin[i][j]);
      Serial.print("\n");
      Serial.print(" MaxLight: ");
      Serial.print(lightMax[i][j]);
      Serial.print("\n");      
    }
  }


  // TODO, maybe elsewhere
  handleChange(0);
  handleChange(1);
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
      lightMapped[i][j] = map(lightValue[i][j], max(lightMin[i][j] - 20, 0), min(lightMax[i][j] + 20, 1023), 0, 255);
    }
  }

  /*for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      Serial.print("In junction ");
      Serial.print(i);
      Serial.print(" LightMapped: ");
      Serial.println(lightMapped[i][j]);   
    }
  }*/

  for (int i = 0; i < 2; i++) 
  {
    if ((lightMapped[i][0] < 125) && !carInJunction[i][0]) 
    {
      cars[i][0]++;
      carInJunction[i][0] = true;
    }
    else if (lightMapped[i][0] >= 190)
      carInJunction[i][0] = false;

    if ((lightMapped[i][1] < 140) && !carInJunction[i][1]) 
    {
      cars[i][1]++;
      carInJunction[i][1] = true;
    }
    else if (lightMapped[i][1] >= 180)
      carInJunction[i][1] = false;
  }
}

void calculateDutyCycle(int junction) 
{
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

void lightCalibration() 
{
  for (int i = 0; i < 2; i++) 
  {
    for (int j = 0; j < 2; j++) 
    {
      lightValue[i][j] = analogRead(light[i][j]);

      if (lightValue[i][j] < lightMin[i][j])
        lightMin[i][j] = lightValue[i][j];
      
      if (lightValue[i][j] > lightMax[i][j])
        lightMax[i][j] = lightValue[i][j];
    }
  }
}