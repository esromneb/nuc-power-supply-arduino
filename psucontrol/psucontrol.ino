
// #define USE_DEBUG_BUTTON

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
const int STAY_ON_SWITCH_PIN = 10; // allow reboots in this messed up system

#ifdef USE_DEBUG_BUTTON
const int ADJUST_PIN = 12;
#endif


#define sprint Serial.println


// off time PSU needs to be off during a reboot
// const int PSU_OFF_REBOOT_TIME = 500;
int PSU_OFF_REBOOT_TIME = 6000;

// #define VERBOSE_FAN_CURRENT

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
  delay(75);
  fan(0);
  delay(75);
  fan(1);
  delay(75);
  fan(0);
  delay(75);
  fan(1);


  pinMode(STAY_ON_SWITCH_PIN, INPUT_PULLUP);

  
#ifdef USE_DEBUG_BUTTON
  pinMode(ADJUST_PIN, INPUT_PULLUP);
#endif
  
  // digitalWrite(13, LOW);

  // give me a chance to open the monitor
  // delay(1500);
  Serial.begin(115200);
}



#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


typedef void (*Runnable)(unsigned long);

#define STATE_OFF_FRESH 0
#define STATE_OFF_ASK 1
#define STATE_OFF_ASK_2 2
#define STATE_OFF_WAIT 3
#define STATE_CPU_ON 4

int cstate = STATE_OFF_FRESH;

void ps(void) {
  switch (cstate) {
  case STATE_OFF_FRESH:
    Serial.println("STATE_OFF_FRESH");
    break;
  case STATE_OFF_ASK:
    Serial.println("STATE_OFF_ASK");
    break;
  case STATE_OFF_ASK_2:
    Serial.println("STATE_OFF_ASK_2");
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
unsigned long waitc = 0;
unsigned long waitd = 0;
int time_read_off = 0;
int waitd_oneshot = 0;

void power_sequence(const unsigned long now) {


  switch (cstate) {
  case STATE_OFF_FRESH:
    ps();
    
    // If asked to power on
    if(!digitalRead(STAY_ON_SWITCH_PIN)) {
      waita = now;
      psu_control(true);
      sprint("Stay-ON at first power");
      cstate = STATE_OFF_WAIT;
      ps();
    } else {
      // asked to power off
      sprint("Stay-OFF at first power");
      cstate = STATE_OFF_ASK_2;
      ps();
      psu_control(false); // redundant
      waitd_oneshot = 0; // don't check for glitch
    }
    
    break;
  case STATE_OFF_ASK:

    // during state off ASK
    // it's possible that the CPU is doing a reboot
    // in this case we need to wait a short bit, then turn the PSU on
    // if the CPU doesn't come up afterwards after 30 seconds, then PSU goes off (cpu is really off)
    if( (now-waitc) < PSU_OFF_REBOOT_TIME ) {
      break;
    }
    // Serial.println("glitch PSU on");
    psu_control(false); // redundant
    waitd = now;
    cstate = STATE_OFF_ASK_2;
    waitd_oneshot = 1;
    ps();

    break;

  case STATE_OFF_ASK_2:
    // when here, we've pulsed the PSU off, then back on
    // now we need to wait and decide if it should ultimately be turned off

    // if( (now-waitd) > 15000 && waitd_oneshot == 1) {
    //   // cpu is actually off
    //   // PSU off
    //   psu_control(false);
    //   // delay(PSU_OFF_REBOOT_TIME); // prevent case where button is pressed on this frame
    //   delay(2000); // prevent case where button is pressed on this frame
    //   // in this case we need the same delay as before
    //   waitd_oneshot = 0;
    //   Serial.println("system really was off, turning off PSU");
    // }



    // what happens if cpu is already on?
    // waita = now;
    // psu_control(true);
    // cstate = STATE_OFF_WAIT;

    // if the "stay on" switch is active
    if(!digitalRead(STAY_ON_SWITCH_PIN)) {
      cstate = STATE_OFF_WAIT;
      psu_control(true);
      ps();
    }

    // if you pressed the button
    if(!digitalRead(POWER_BUTTON_PIN)) {
      cstate = STATE_OFF_WAIT;
      psu_control(true);
      ps();
    }

    // cpu turned on, it was probably a reset
    if(digitalRead(NUC_USB_PIN)) {
      cstate = STATE_CPU_ON;
      waitb = now;
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
    if( (now-waitb) < 5000 ) {
      break;
    }

    // true means press
    // const bool human_button_press = !digitalRead(POWER_BUTTON_PIN);

    // true will press
    // pass the button through
    nuc_power_button(!digitalRead(POWER_BUTTON_PIN));

    if(!digitalRead(NUC_USB_PIN)) {
      Serial.println("turning PSU off");
      psu_control(false);
      cstate = STATE_OFF_ASK;
      ps();
      // delay(300);
      while(!digitalRead(POWER_BUTTON_PIN)) {
        Serial.println("blocking while button is held");
      }
      waitc = millis();
      // ps();
    }

    break;
  
  default:
    break;
  }



  // true if cpu is on
  // const bool cpu = digitalRead(NUC_USB_PIN);
  // Serial.println(String(cpu));

  

}


// lower value is more current
// around 512 it "no current"
// however I've sen 506-512 values in this state
const int thresh = 498;

unsigned long last_hdd_on = 0; // time that we've last seen the hdd's on
unsigned long hdd_fan_coast = 60000; // time that fans stay on beyond hdd spindown
int last_fan = 1;
int fan_one_click = 1;

void read_hdd_current(const unsigned long now) {
  // Serial.println("Function 1 called at time: " + String(now));

  // get drive current
  const int current = analogRead(CURRENT_PIN);

  #ifdef VERBOSE_FAN_CURRENT
  Serial.print("Fan Current " + String(current) + ": ");
  #endif

  if( current <= thresh ) {
    last_hdd_on = now;
    fan(1); // Fans on
    if( 1 != last_fan) {
      Serial.println("Fans turned on with a current of: " + String(current));
    }
    last_fan = 1;
    fan_one_click = 1;
  }

  // could go in an else, but functionally is in an "else" due to value of last_hdd_on
  if( now - last_hdd_on >= hdd_fan_coast ) {
    fan(0); // Fans off
    if( 0 != last_fan) {
      Serial.println("Fans turned off with a current of: " + String(current));
    }
    last_fan = 0;

// #ifdef VERBOSE_FAN_CURRENT
    // Serial.println("Fans turned off at time: " + String(now));
// #endif
  } else if (current > thresh) {
    Serial.println("Fans Coasting");
    
    // notify that we know that power is off
    // but still coast
    if(fan_one_click) {
      fan_one_click = 0;
      fan(0);
      delay(50);
      fan(1);
    }
  }

  #ifdef VERBOSE_FAN_CURRENT
    sprint(""); // get the newline for above
  #endif

}

void function2(const unsigned long now) {
  // Code for function 2
  // Do something with the time value
  Serial.println("Function 2 called at time: " + String(now));
}

#ifdef USE_DEBUG_BUTTON
void debug_button(const unsigned long now) {
  static bool button_previous = 1;
  int current = digitalRead(ADJUST_PIN);
  

  if( current && current != button_previous) {
    PSU_OFF_REBOOT_TIME += 1000;
    if( PSU_OFF_REBOOT_TIME > 10000) {
      PSU_OFF_REBOOT_TIME = 1000;
    }
    Serial.println("New delay " + String(PSU_OFF_REBOOT_TIME));
  }

  button_previous = current;


}
#endif

// Array of function pointers
// Runnable functions[] = {power_sequence, debug_button};
// Runnable functions[] = {power_sequence};
Runnable functions[] = {power_sequence, read_hdd_current};
// Runnable functions[] = {read_hdd_current};
// Runnable functions[] = {read_hdd_current, function2};

const int num_runnable = ARRAY_SIZE(functions);

// Array of last run times
unsigned long last_run[] = {0, 0};

// Array of periods
unsigned long period[] = {10, 1000, 99999};



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
