// List of LED pins
const int LEDs[] = {5, 4, 3, 6}; // Array of LED pins
const int buttonPin = 2;

volatile bool isFrozen = false;
const int second = 1000;
unsigned long time_now = 0;

void setup() {
  // LEDs initialization
  for (int i = 0; i < sizeof(LEDs); i++) {
    pinMode(LEDs[i], OUTPUT);
  }
  // Button initializion
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, FALLING); // Interrupts when button is pressed
}

void loop() {
  // Cycle of 4 seconds (LEDs)
  for (int i = 0; i < sizeof(LEDs); i++) { // Iterate through LEDs
    digitalWrite(LEDs[i], HIGH); // Turn on LED
    time_now = millis();
    // delays for 1 second
    while(millis() < time_now + second){
      while (isFrozen) { // Check pause flag
        delay(10); // Delay to avoid excessive CPU 
      }
    }    
    digitalWrite(LEDs[i], LOW); // Turn off LED
  }
  // Last second of the 5 second cycle
  while(millis() < time_now + second){
      while (isFrozen) { // Check pause flag
        delay(10); // Delay to avoid excessive CPU 
      }
  }
}

void buttonInterrupt() { // Interrupt handler for button press
  isFrozen = !isFrozen; // Toggle pause state
}