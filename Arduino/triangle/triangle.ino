/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
    for (int i=11;i<=13;i++) {
	pinMode(i, OUTPUT);     
    }
}

// the loop routine runs over and over again forever:
void loop() {
    digitalWrite(11, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(12, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(13, HIGH);    // turn the LED off by making the voltage LOW
    delay(500);                  // wait for a second
    digitalWrite(11, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(500);                  // wait for a second
}
