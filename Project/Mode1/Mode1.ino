#include <Wire.h>

// User variables
int coordinate = 1;
int allCoordinates[2] = {0, 1};
int activeMode = 2;
// End of user variables

// Coordinates
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
const unsigned int maxSpeed = 40;

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
  // ISR should not have Wire library calls in this system
  // These calls are too slow and cause trouble
  // The use of a data received flag makes it possible to handle
  // Outside of the ISR
  dataReceived = true;
}

// Data Handler (round robin)
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
      car();
      break;
    case 2:
    // Mode
      mode();
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
  dataReceived = false;
}

//
// Receiving functions
//
void clock(int destination, int source)
{ 
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

  if (source == coordinate - 1)
  {
    westClock = receivedClock;
  }
  else if (source == coordinate + 1)
  {
    eastClock = receivedClock;
  }
  
  timeNow = millis() / 100;
  long unsigned int myNewClock = 0;
  if (eastClock && westClock)
  {
    myNewClock = ((timeNow + deltaClock) + eastClock + westClock) / 3;
  }
  else if (coordinate == 0 && eastClock)
  {
    myNewClock = ((timeNow + deltaClock) + eastClock) / 2;
  }
  else if (coordinate == highestCoordinate && westClock)
  {
    myNewClock = ((timeNow + deltaClock) + westClock) / 2;
  }
  condition = (timeNow + deltaClock) - myNewClock;
  condition = abs(condition);
  if (condition < error)
  {
    stopSync = true;
    eastClock = 0;
    westClock = 0;
    Serial.println("Done!!");
    if (coordinate == 0)
    {
      doneCounter++;
    }
    else
    {
      syncDone();
    }
    return;
  }
  deltaClock = myNewClock - timeNow;
}

void car()
{
  long unsigned int receivedClock = 0;
  long unsigned int timeNow = 0;
  long int condition = 0;
  // 32 bits -> 4 bytes
  int received;
  Serial.println("Car received");
  while (Wire.available())
  {
    received = Wire.read();
    receivedClock = receivedClock << 8;
    receivedClock += received;
  }

  // A controller only receives from the
  circularBuffer[0][tail[0]] = receivedClock;
  tail[0] = (tail[0] + 1) % 32;
}

void mode()
{
  int modeData;
  int received = 0;
  if (Wire.available())
  {
    received = Wire.read();
    activeMode = received;
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
      Serial.println("Entered sync");
      doneCounter++;
    }
  }
  // Ack
  else if (syncData == 0x02)
  {
    synchronized = true;
  }
}

//
// Sending functions
//
void clockSync()
{
  long int myClock = 0;
  long unsigned int timeNow;
  while (!synchronized)
  {    
    timeNow = millis();
    while (millis() < timeNow + 100)
    {
      if (dataReceived)
        dataHandler();
    }
    Serial.print("my clock: ");
    myClock = (millis() / 100) + deltaClock;
    Serial.println(myClock);
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
      // Data (TIME - 32 bit integer)
      Wire.write(parseTime());
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
      // Event (CLOCK)
      int event = 0x00;
      Wire.write(event);
      // Data (TIME - 32 bit integer)
      Wire.write(parseTime());
      Wire.endTransmission();
    }

    // Check if synchronization has ended
    if (coordinate == 0 && doneCounter == highestCoordinate + 1)
    {
      synchronized = true;
      // Node 0 sends sync ack to all other nodes and the current mode
      syncAck();
      sendMode();
    } 
  }
}

void carThrough(int coordinateToSend)
{
  Wire.beginTransmission(coordinateToSend);
  // Addressing block
  int address = coordinateToSend << 4;
  Wire.write(address);
  // Source
  int source = coordinate << 4;
  Wire.write(source);
  // Event (CAR)
  Wire.write(0x01);
  // Data (TIME - 32 bit integer)
  Wire.write(parseTime());
  Wire.endTransmission();
  Serial.println("Car through");
}

void sendMode()
{
  for(int i = 1; i < highestCoordinate + 1; i++)
  {
    Wire.beginTransmission(i);
    // Destination
    Wire.write(i << 4);
    // Source
    Wire.write(coordinate << 4);
    // Event (MODE)
    Wire.write(0x02);
    // Data (ACK)
    Wire.write(activeMode);
    Wire.endTransmission();
    Serial.println("Mode sent");
  }
}

// When in sync send a Done to coordinate 0
void syncDone()
{
  //Serial.println("ANTES");
  Wire.beginTransmission(0);
  // Destination
  Wire.write(0x00);
  // Source
  Wire.write(coordinate << 4);
  // Event (SYNC)
  Wire.write(0x04);
  // Data (DONE)
  Wire.write(0x01);
  Wire.endTransmission();
  Serial.println("Sync Done sent");
}

void syncAck()
{
  for(int i = 1; i < highestCoordinate + 1; i++)
  {
    Wire.beginTransmission(i);
    // Destination
    Wire.write(i << 4);
    // Source
    Wire.write(coordinate << 4);
    // Event (SYNC)
    Wire.write(0x04);
    // Data (ACK)
    Wire.write(0x02);
    Wire.endTransmission();
    Serial.println("Sync Ack sent");
  }
}

// This function assumes that the caller is in 
// the middle of writting a transmission
long unsigned int parseTime()
{
  // Data (TIME - 32 bit integer)
  long unsigned int data = (millis() / 100) + deltaClock;
  long unsigned int parsedData = 0;
  for (int i = 3; i >= 0; i--)
  {
    parsedData = parsedData + data >> 8 * i;
  }
  return parsedData;
}

// Other Functions
void loop() 
{
  if (dataReceived)
    dataHandler();

  if (activeMode != 0)
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
            if(!activeMode == 0)
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
      carInJunction[i][0] = true;

      if (activeMode == 2)
      {
        // Speed related
        if (head[i] < tail[i])
        {
          // Verifies timestamp in self buffer to see if the speed limit was exceded
          if (circularBuffer[i][head[i]] + maxSpeed > (millis() / 100) + deltaClock)
          {
            stop(i);
          }
          // Head (read pointer) increaments
          head[i] = (head[i] + 1) % 32;
        }
        
        // First junction writes in the second junction
        if (i == 0)
        {
          circularBuffer[i + 1][tail[i + 1]] = (millis() / 100) + deltaClock;
          tail[i + 1] = (tail[i + 1] + 1) % 32;
        }

        // Second junction sends to next controller
        else if (i == 1)
        {
          // send to the next arduino
          if (coordinate != highestCoordinate)
          {
            carThrough(coordinate + 1);
          }
        }
      }
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