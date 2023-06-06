
// 13 output to relay
// 12 is input from fan
// 2 is a copy fo the 5v (wired to 5v from the 2nd usb (not the one onboard the arduino))
// 3 is the relay to control the fans
// A1 is the input from the current sensor

const int CURRENT_PIN = A1;
const int FAN_RELAY_PIN = 3;

#define VERBOSE


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW); // Fans on
  
  digitalWrite(13, LOW);

  delay(1000);
  Serial.begin(115200);
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

void loop2() {
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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// void loop3() {
//   int fan = analogRead(A0);
//   Serial.println(fan);
//   delay(75);
// }

typedef void (*Runnable)(unsigned long);

const int thresh = 506;
unsigned long last_hdd_on = 0; // time that we've last seen the hdd's on
unsigned long hdd_fan_coast = 3000; // time that fans stay on beyond hdd spindown

void read_hdd_current(const unsigned long now) {
  // Code for function 1
  // Do something with the time value
  // Serial.println("Function 1 called at time: " + String(now));
  const int current = analogRead(CURRENT_PIN);

  if( current <= thresh ) {
    last_hdd_on = now;
    digitalWrite(FAN_RELAY_PIN, LOW); // Fans on
#ifdef VERBOSE
    Serial.println("Fans turned on with a current of: " + String(current));
#endif
  }

  if( now - last_hdd_on >= hdd_fan_coast ) {
    digitalWrite(FAN_RELAY_PIN, HIGH); // Fans off
#ifdef VERBOSE
    Serial.println("Fans turned off at time: " + String(now));
#endif
  }
}

void function2(const unsigned long now) {
  // Code for function 2
  // Do something with the time value
  Serial.println("Function 2 called at time: " + String(now));
}

// Array of function pointers
Runnable functions[] = {read_hdd_current};
// Runnable functions[] = {read_hdd_current, function2};

const int num_runnable = ARRAY_SIZE(functions);

// Array of last run times
unsigned long last_run[] = {0, 0};

// Array of periods
unsigned long period[] = {1000, 99999};



void loop() {
  const unsigned long now = millis();

  for (int i = 0; i < num_runnable; i++) {
    if (now - last_run[i] >= period[i]) {
      functions[i](now);
      last_run[i] = now;
    }
  }
}

void loop4() {
  const unsigned long now = millis();
  int current = analogRead(A1);
  Serial.println(current);
  Serial.print("time: ");
  Serial.println(now);
  delay(75);
}



// void loop() {
//   digitalWrite(FAN_RELAY_PIN, LOW); // Fans on
//   delay(1000);
//   digitalWrite(FAN_RELAY_PIN, HIGH); // Fans off
//   delay(4000);
// }
