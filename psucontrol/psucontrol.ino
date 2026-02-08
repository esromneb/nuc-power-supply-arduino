
// #define USE_DEBUG_BUTTON

// A1 is the input from the current sensor

const int CURRENT_PIN = A1; // drive current pin
const int FAN_RELAY_PIN = 3; // output to control the fan relay
const int PSU_CONTROL_PIN = 9; // Control green "Switch" line on the PSU

#ifdef USE_DEBUG_BUTTON
const int ADJUST_PIN = 12;
#endif


#define sprint Serial.println


#define VERBOSE_FAN_CURRENT

void fan(bool on) {
  // LOW is on
  digitalWrite(FAN_RELAY_PIN, !on);
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

  // Turn PSU on and leave it on
  psu_control(true);

#ifdef USE_DEBUG_BUTTON
  pinMode(ADJUST_PIN, INPUT_PULLUP);
#endif

  Serial.begin(115200);
}



#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

typedef void (*Runnable)(unsigned long);


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


#ifdef USE_DEBUG_BUTTON
void debug_button(const unsigned long now) {
  static bool button_previous = 1;
  int current = digitalRead(ADJUST_PIN);
  
  if( current && current != button_previous) {
    Serial.println("Debug button pressed");
  }

  button_previous = current;
}
#endif

// Array of function pointers
Runnable functions[] = {read_hdd_current};

const int num_runnable = ARRAY_SIZE(functions);

// Array of last run times
unsigned long last_run[] = {0};

// Array of periods
unsigned long period[] = {1000};



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
