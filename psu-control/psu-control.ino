/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

*/

// 13 output to relay
// 12 is input from fan
// 2 is a copy fo the 5v (wired to 5v from the 2nd usb (not the one onboard the arduino))

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  
  digitalWrite(13, LOW);

  delay(1000);
  // Serial.begin(9600);
}


// the loop function runs over and over again forever
// void loop() {
//   digitalWrite(13, HIGH);  // turn the LED on (HIGH is the voltage level)
//   delay(3000);                      // wait for a second
//   digitalWrite(13, LOW);   // turn the LED off by making the voltage LOW
//   delay(250);                      // wait for a second
// }

// 0 was off
// 1 was on
int state = 0;

void loop() {
  while(1) {
    if(state == 0) {
      int fan = analogRead(A0);
      if(fan > 15) {
        digitalWrite(13, HIGH);
        state = 1;
        break;
      }
    }
    if(state == 1) {
      int fan = analogRead(A0);
      int vin = digitalRead(2);
      if( (fan <= 15) || (vin == 0) ) {
        digitalWrite(13, LOW);
        state = 0;
        delay(1000);
        break;
      }
    }
  }
}

// void loop() {
//   int fan = analogRead(A0);
//   Serial.println(fan);
//   delay(75);
// }
