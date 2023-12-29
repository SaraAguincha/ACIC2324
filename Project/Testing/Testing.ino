// List of LED pins
const int led[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // Array of LED pins
const int lSensor[] = {A0, A1, A2, A3, A4};

void setup() {
  // LEDs initialization
  Serial.begin(9600);
  for (int i = 0; i < sizeof(led) / sizeof(led[0]); i++) 
  {
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], HIGH);
  }

  for (int i = 0; i < sizeof(lSensor) / sizeof(lSensor[0]); i++)
  {
    pinMode(lSensor[i], INPUT);
  }
}
  

void loop() {
  for (int i = 0; i < sizeof(lSensor) / sizeof(lSensor[0]); i++)
  {
    Serial.print("Light sensor number: ");
    Serial.print(i);
    Serial.print(" has value: ");
    Serial.print(analogRead(lSensor[i]));
    Serial.println("");
  }
  Serial.println("");
  delay(3000);
}