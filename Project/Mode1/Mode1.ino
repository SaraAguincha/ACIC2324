#include <Wire.h>

// Coordinates
int coordinate = 1;
int allCoordinates[2] = {0, 1};
int highestCoordinate = (sizeof(allCoordinates) / sizeof(allCoordinates[0])) - 1;

// Initialization
const int junctionLEDs[2][2][3] = {{{13, 12, 11}, {8, 10, 9}}, {{7, 6, 5}, {2, 4, 3}}};

int circularBuffer[2][32];
int head[2] = {0, 0};
int tail[2] = {0, 0};
// Time
long unsigned int time_now[] = {0, 0};
long unsigned int timeZero[] = {0, 0};
long unsigned int period = 200;
float dutyCycle[] = {0.5, 0.5};
bool westAllowed[] = {true, true};
bool isChanging[] = {true, true};

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

// Speeding stuff
const unsigned int maxSpeed = 4000;

// Clock
long int deltaClock = 0;
long unsigned int error = 1; // 100 ms unit 
bool synchronized = false;
bool stopSync = false;
long unsigned int westClock = 0;
long unsigned int eastClock = 0;
int doneCounter = 0;

bool dataReceived = false;

// TODO - Eventually do this - turns yellow after red
bool forcedStop[2] = {false, false};

void setup() 
{
  Serial.begin(9600);
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
  // I2C Communication
  Wire.begin(coordinate);
  Wire.onReceive(receiveEvent);
  // Clock synchronization
  clockSync();
  // Start of the system
  initialState();
  timeZero[0] = (millis() / 100) + deltaClock;
  timeZero[1] = (millis() / 100) + deltaClock;
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  dataReceived = true;
}
//
// Receiving functions
//

// Event Received
void clock(int destination, int source)
{
  /*Serial.print("delta clock ");
  Serial.println(deltaClock);

  Serial.print("my clock: ");
  long int myCock = millis() / 100 + deltaClock;
  Serial.println(myCock);*/
  

  long unsigned int receivedClock = 0;
  long unsigned int timeNow = 0;
  long int condition = 0;
  // 32 bits -> 4 bytes
  int received;
  while (Wire.available())
  {
    received = Wire.read();
    receivedClock = receivedClock << 8;
    receivedClock += received;
  }

  if (stopSync)
    return;
  //Serial.print("ReceivedClock ");
  //Serial.println(receivedClock);

  if (source == coordinate - 1)
  {
    westClock = receivedClock;
  }
  else if (source == coordinate + 1)
  {
    eastClock = receivedClock;
  }

  
  /*Serial.println(eastClock);
  Serial.println(westClock);
  Serial.println(" ");*/

  //Serial.println(deltaClock);

  if (eastClock && westClock)
  {
    timeNow = millis() / 100;
    long unsigned int myNewClock = ((timeNow + deltaClock) + eastClock + westClock) / 3;
    condition = (timeNow + deltaClock) - myNewClock;
    condition = abs(condition);
    if (condition < error)
    {
      stopSync = true;
      eastClock = 0;
      westClock = 0;
      Serial.println("Done!!");
      sendDone();
      return;
    }
    deltaClock = myNewClock - timeNow;
  }
  else if (coordinate == 0 && eastClock)
  {
    timeNow = millis() / 100;
    long unsigned int myNewClock = ((timeNow + deltaClock) + eastClock) / 2;
    condition = (timeNow + deltaClock) - myNewClock;
    condition = abs(condition);
    if (condition < error)
    {
      stopSync = true;
      eastClock = 0;
      doneCounter++;
      Serial.println("Done!!");
      return;
    }
    deltaClock = myNewClock - timeNow;
  }
  else if (coordinate == highestCoordinate && westClock)
  {
    timeNow = millis() / 100;
    long unsigned int myNewClock = ((timeNow + deltaClock) + westClock) / 2;
    
    condition = (timeNow + deltaClock) - myNewClock;
    condition = abs(condition);
    if (condition < error)
    {
      stopSync = true;
      westClock = 0;
      Serial.println("Done!!");
      sendDone();
      return;
    }
    deltaClock = myNewClock - timeNow;
  }
}

// Send Sync Done, and Receive Sync Ack
void sync()
{
  int syncData;
  int received = 0;
  if (Wire.available())
  {
    received = Wire.read();
    syncData = received;
    //Serial.println(syncData);
  }
  // Done
  if (syncData == 0x01)
  {
    if (coordinate == 0)
    {
      doneCounter++;
      if (doneCounter == highestCoordinate + 1)
      {
        synchronized = true;
        for(int i = 0; i < highestCoordinate + 1; i++)
        {
          Wire.beginTransmission(i);
          // Destination
          Wire.write(i << 4);
          // Source
          Wire.write(coordinate << 4);
          // Event
          Wire.write(0x04);
          // Data
          Wire.write(0x02);
          Wire.endTransmission();
        }
      } 
    }
  }
  // Ack
  else if (syncData == 0x02)
  {
    synchronized = true;
  }
}

// When in sync send a Done to coordinate 0
void sendDone()
{
  //Serial.println("ANTES");
  Wire.beginTransmission(0);
  // Destination
  Wire.write(0x00);
  // Source
  Wire.write(coordinate << 4);
  // Event
  Wire.write(0x04);
  // Data
  Wire.write(0x01);
  Wire.endTransmission();
  Serial.println("DEPOIS");
}

//
// Sending functions
//
void clockSync()
{
  long unsigned int data;
  long int myClock = 0;
  while (!synchronized)
  {    
    if (dataReceived)
    {
      dataHandler();
    }
    //Serial.print("my clock: ");
    //myClock = (millis() / 100) + deltaClock;
    //Serial.println(myClock);
    if (coordinate != 0)
    {
      Wire.beginTransmission(coordinate - 1);
      // Addressing block
      int address = (coordinate - 1) << 4;
      Wire.write(address);
      // Source
      int source = coordinate << 4;
      Wire.write(source);
      // Event
      int event = 0x00;
      Wire.write(event);
      // Data 
      data = (millis() / 100) + deltaClock;
      for (int i = 3; i >= 0; i--)
      {
        Wire.write(data >> 8 * i);
      }
      Wire.endTransmission();
    }
    if (coordinate != highestCoordinate)
    {
      Wire.beginTransmission(coordinate + 1);
      // Addressing block
      int address = (coordinate + 1) << 4;
      Wire.write(address);
      // Source
      int source = coordinate << 4;
      Wire.write(source);
      // Event
      int event = 0x00;
      Wire.write(event);
      // Data 
      data = (millis() / 100) + deltaClock;
      for (int i = 3; i >= 0; i--)
      {
        Wire.write(data >> 8 * i);
      }
      Wire.endTransmission();
    }
    delay(100);
  }
}

void dataHandler()
{
  int received;
  int destination;
  int source;
  int event;
  // loop through all but the last
  if (Wire.available())
  {
    received = Wire.read();
    destination = received >> 4;
  }

  if (Wire.available())
  {
    received = Wire.read();
    source = received >> 4;
  }

  if (Wire.available())
  {
    received = Wire.read();
    event = received;
  }

  //Serial.print("Received event: ");
  //Serial.println(event);

  // Round robin with interrupts
  switch (event)
  {
    // Clock
    case 0:
      //clock = true;
      clock(destination, source);
      break;
    // Car
    case 1:
      
      break;
    case 2:
    // Mode
      
      break;
    case 3:
    // Status
      
      break;
    case 4:
    // Sync
      sync();
      break;

    default:
      break;
  }
}

// Other Functions
void loop() 
{
  readCars();
  for (int i = 0; i < 2; i++) 
  {
    if (((millis() / 100) + deltaClock) > timeZero[i] + 10) 
    {
      if (((millis() / 100) + deltaClock) > timeZero[i] + (period * dutyCycle[i]))
      {
        if (((millis() / 100) + deltaClock) > timeZero[i] + (period * dutyCycle[i]) + 10)
        {
          if (((millis() / 100) + deltaClock) > timeZero[i] + period)
          {
            handleYellow(i);
            timeZero[i] = millis() / 100 + deltaClock;
            calculateDutyCycle(i);
            printStatus(i);
            
            cars[i][0] = 0;
            cars[i][1] = 0;
            isChanging[i] = false;
          }
          else if (!isChanging[i])
          {
            isChanging[i] = !isChanging[i];
            handleChange(i);
          }
        }
        else if (isChanging[i])
        {
          isChanging[i] = !isChanging[i];
          handleYellow(i);
        }
      }
      else if (!isChanging[i])
      {
        isChanging[i] = !isChanging[i];
        handleChange(i);
      }
    }
    else if (isChanging[i])
    {
      isChanging[i] = !isChanging[i];
      handleYellow(i);
    }
  }
}

void printStatus(int junction)
{
  Serial.print("Cars in junction ");
  Serial.print(junction);
  Serial.print(": (West) ");
  Serial.print(cars[junction][0]);
  Serial.print(", (South) ");
  Serial.println(cars[junction][1]);
  Serial.println("\n");

  Serial.print("Duty cycle: ");
  Serial.println(dutyCycle[junction]);
}

// TODO, initialize the RED and GREEN light according to the flag
void initialState () {
  int counter = 0;
  long unsigned int init_time = 0;
  bool blink = true;
  init_time = (millis() / 100) + deltaClock;
  while (counter < 6) {

    lightCalibration();

    if((millis() / 100) + deltaClock > init_time + 10)
    {
      // Yellow lights
      for (int i = 0; i < 2; i++)
      {
        digitalWrite(junctionLEDs[i][0][1], blink ? HIGH : LOW);
        digitalWrite(junctionLEDs[i][1][1], blink ? HIGH : LOW);
      }
      counter++;
      init_time = (millis() / 100) + deltaClock;
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
}

void handleYellow(int junction) {
  if (westAllowed[junction]) {
    digitalWrite(junctionLEDs[junction][0][0], LOW);
    digitalWrite(junctionLEDs[junction][1][2], LOW);
    
  }
  else {
    digitalWrite(junctionLEDs[junction][0][2], LOW);
    digitalWrite(junctionLEDs[junction][1][0], LOW);
  }
  digitalWrite(junctionLEDs[junction][0][1], HIGH);
  digitalWrite(junctionLEDs[junction][1][1], HIGH);
  westAllowed[junction] = !westAllowed[junction];
}

void handleChange(int junction){
  // turns off the yellow lights in both junctions
  digitalWrite(junctionLEDs[junction][0][1], LOW);
  digitalWrite(junctionLEDs[junction][1][1], LOW);

  // if true, west starts green and south red
  if (westAllowed[junction]) {
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

  for (int i = 0; i < 2; i++) 
  {
    if ((lightMapped[i][0] < 125) && !carInJunction[i][0]) 
    {
      cars[i][0]++;

      if (head[i] < tail[i])
      {
        if (circularBuffer[i][head[i]] + maxSpeed > (millis() / 100) + deltaClock)
        {
          stop(i);
        }
        head[i] = (head[i] + 1) % 32;
      }
      
      if (i == 0)
      {
        circularBuffer[i + 1][tail[i + 1]] = (millis() / 100) + deltaClock;
        tail[i + 1] = (tail[i + 1] + 1) % 32;
      }

      else if (i == 1)
      {
        // send to the next arduino
      }

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

void stop(int junction)
{
  timeZero[junction] = (millis() / 100) + deltaClock;
  westAllowed[junction] = true;
  isChanging[junction] = true;
  forcedStop[junction] = true;
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