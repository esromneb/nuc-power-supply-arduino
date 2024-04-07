
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

  // give me a chance to open the monitor
  delay(1500);
  Serial.begin(115200);
}



#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


typedef void (*Runnable)(unsigned long);

#define STATE_OFF_FRESH 0
#define STATE_OFF_ASK 1
#define STATE_OFF_WAIT 2
#define STATE_CPU_ON 3

int cstate = STATE_OFF_FRESH;

void ps(void) {
  switch (cstate) {
  case STATE_OFF_FRESH:
    Serial.println("STATE_OFF_FRESH");
    break;
  case STATE_OFF_ASK:
    Serial.println("STATE_OFF_ASK");
    break;
  case STATE_OFF_WAIT:
    Serial.println("STATE_OFF_WAIT");
    break;
  case STATE_CPU_ON:
    Serial.println("STATE_CPU_ON");
    break;
  
  default:
    break;
  }
}

unsigned long waita = 0;
unsigned long waitb = 0;
int time_read_off = 0;

void power_sequence(const unsigned long now) {


  switch (cstate) {
  case STATE_OFF_FRESH:
    ps();
    // what happens if cpu is already on?
    waita = now;
    psu_control(true);
    cstate = STATE_OFF_WAIT;
    ps();
    break;
  case STATE_OFF_ASK:
    // what happens if cpu is already on?
    // waita = now;
    // psu_control(true);
    // cstate = STATE_OFF_WAIT;
    if(!digitalRead(POWER_BUTTON_PIN)) {
      cstate = STATE_OFF_FRESH;
      ps();
    }
    break;
  
  case STATE_OFF_WAIT:
    if( (now-waita) > 900 ) {
      // turn the nuc on
      // this could be broken out into new states
      Serial.println("Pressing nuc to turn it on");
      nuc_power_button(true);
      delay(100);
      nuc_power_button(false);
      cstate = STATE_CPU_ON;
      waitb = now;
      ps();
    }
    break;

  case STATE_CPU_ON:
    // two second lockout before this state will run
    // waitb must be set before asking to switch to STATE_CPU_ON
    if( (now-waitb) < 2000 ) {
      break;
    }

    // true means press
    // const bool human_button_press = !digitalRead(POWER_BUTTON_PIN);

    // true will press
    // pass the button through
    nuc_power_button(!digitalRead(POWER_BUTTON_PIN));

    if(!digitalRead(NUC_USB_PIN)) {
      cstate = STATE_OFF_ASK;
      ps();
      delay(300);
      while(!digitalRead(POWER_BUTTON_PIN)) {
        Serial.println("blocking while button is held");
      }
    }

    break;
  
  default:
    break;
  }



  // true if cpu is on
  // const bool cpu = digitalRead(NUC_USB_PIN);
  // Serial.println(String(cpu));

  

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

// void loop4() {
//   const unsigned long now = millis();
//   int current = analogRead(A1);
//   Serial.println(current);
//   Serial.print("time: ");
//   Serial.println(now);
//   delay(75);
// }



// void loop() {
//   digitalWrite(FAN_RELAY_PIN, LOW); // Fans on
//   delay(1000);
//   digitalWrite(FAN_RELAY_PIN, HIGH); // Fans off
//   delay(4000);
// }
