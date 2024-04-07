
// 13 output to relay
// 12 is input from fan
// 2 is a copy fo the 5v (wired to 5v from the 2nd usb (not the one onboard the arduino))
// A1 is the input from the current sensor

const int POWER_COPY = 2;
const int CURRENT_PIN = A1; // drive current pin
const int FAN_RELAY_PIN = 3; // output to control the fan relay
const int NUC_USB_PIN = A0; // measure the USB power of the nuc's yellow USB port
const int NUC_POWER_PIN = 6; // output to "press" (control) the power button on the nuc
const int POWER_BUTTON_PIN = 4; // Human press this to turn the system on
const int PSU_CONTROL_PIN = 9; // Control green "Switch" line on the PSU

#define VERBOSE

void fan(bool on) {
  // LOW is on
  digitalWrite(FAN_RELAY_PIN, !on);
}

// true means "press"
// false means "unpress"
void nuc_power_button(bool press) {
  if(press) {
    pinMode(NUC_POWER_PIN, OUTPUT);
    digitalWrite(NUC_POWER_PIN, LOW);
  } else {
    pinMode(NUC_POWER_PIN, INPUT);
  }
}

// true means "power on (connect)"
// false means "power off (unconnect)"
void psu_control(bool power) {
  if(power) {
    pinMode(PSU_CONTROL_PIN, OUTPUT);
    digitalWrite(PSU_CONTROL_PIN, LOW);
  } else {
    pinMode(PSU_CONTROL_PIN, INPUT);
  }
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  // pinMode(13, OUTPUT);
  // pinMode(POWER_COPY, INPUT);

  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);

  pinMode(NUC_USB_PIN, INPUT);

  pinMode(NUC_POWER_PIN, INPUT);
  nuc_power_button(false);

  pinMode(PSU_CONTROL_PIN, INPUT);
  psu_control(false);

//  psu_control(true);

  pinMode(FAN_RELAY_PIN, OUTPUT);
  fan(1);

  
  
  // digitalWrite(13, LOW);

  // delay(1000);
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
// int state = 0;

// void loop2() {
//   while(1) {
//     if(state == 0) {
//       int fan = analogRead(A0);
//       if(fan > 15) {
//         digitalWrite(13, HIGH);
//         state = 1;
//         break;
//       }
//     }
//     if(state == 1) {
//       int fan = analogRead(A0);
//       int vin = digitalRead(POWER_COPY);
//       if( (fan <= 15) || (vin == 0) ) {
//         digitalWrite(13, LOW);
//         state = 0;
//         delay(1000);
//         break;
//       }
//     }
//   }
// }

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// void loop3() {
//   int fan = analogRead(A0);
//   Serial.println(fan);
//   delay(75);
// }

typedef void (*Runnable)(unsigned long);

#define STATE_OFF_FRESH 0
#define STATE_OFF_WAIT 1

int cstate = STATE_OFF_FRESH;

unsigned long waita = 0;

void power_sequence(const unsigned long now) {


  switch (cstate) {
  case STATE_OFF_FRESH:
    // what happens if cpu is already on?
    waita = now;
    // psu_control(true);

    break;
  
  case STATE_OFF_WAIT:
    if( (now-waita) > 900 ) {

    }
    break;
  
  default:
    break;
  }



  // true if cpu is on
  const bool cpu = digitalRead(NUC_USB_PIN);

  // Serial.println("CPU sees: " + String(cpu));
  Serial.println(String(cpu));

  // true means press
  const bool human_button_press = !digitalRead(POWER_BUTTON_PIN);

  // true will press
  nuc_power_button(human_button_press);

}


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
    fan(1); // Fans on
#ifdef VERBOSE
    Serial.println("Fans turned on with a current of: " + String(current));
#endif
  }

  if( now - last_hdd_on >= hdd_fan_coast ) {
    fan(0); // Fans off
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
Runnable functions[] = {power_sequence};
// Runnable functions[] = {read_hdd_current};
// Runnable functions[] = {read_hdd_current, function2};

const int num_runnable = ARRAY_SIZE(functions);

// Array of last run times
unsigned long last_run[] = {0, 0};

// Array of periods
unsigned long period[] = {10, 99999};



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
