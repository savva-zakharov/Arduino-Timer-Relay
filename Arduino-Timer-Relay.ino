#include <arduino-timer.h>

#include <LiquidCrystal_I2C.h>

#include <Wire.h>

auto timer = timer_create_default();
auto timer_mode8_1 = timer_create_default();
auto timer_mode8_2 = timer_create_default();

// Initialize the LCD, I2C address 0x3F, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Button pins
const int buttonSecondsUp = 2;
const int buttonSecondsDown = 3;
const int pinSwitchDown = 4;
const int pinSwitchUp = 5;
const int startButton = 6;
const int analogPin = A3;

// Output pins

const int relay1pin = 7;
const int relay2pin = 8;

// Mode vals

byte mode = 0, prevMode = 0;

int valSmooth = 0;

bool exposureRunning = false;
bool timer1Running = false;
bool timer2Running = false;

unsigned long exposureTimeTotal;

byte switchState = 0, prevSwitchState = 0;

unsigned long modeState[9] = { 20, 6, 1000, 1000, 1000, 1000, 60000, 60000, 60000 };
String modeLabel[9] = { "dist", "fstop", "expo", "expo", "expo", "expo", "time", "time", "time" };
long exposureStartTime = 0;

int stepScale = 1000;
int stepScaleList[5] = { 1, 10, 100, 1000, 10000 };
const char* fstopList[11] = { "f/1", "f/1.4", "f/2", "f/2.8", "f/4", "f/5.6", "f/8", "f/11", "f/16", "f/22", "f/32" };

int prevDistance = 20;
int prevFstop = 5;

// Previous button states for debouncing
bool prevSecondsUp = false, prevSecondsDown = false, prevSwitchDown = false, prevSwitchUp = false, prevStart = false;
unsigned long lastSecondsUp = 0, lastSecondsDown = 0, lastStart = 0;
const int debounceTime = 200;
bool click = false;

void setup() {
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight

  // Configure button pins
  pinMode(buttonSecondsUp, INPUT_PULLUP);
  pinMode(buttonSecondsDown, INPUT_PULLUP);
  pinMode(pinSwitchDown, INPUT_PULLUP);
  pinMode(pinSwitchUp, INPUT_PULLUP);
  pinMode(startButton, INPUT_PULLUP);
  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);

  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);

  Serial.begin(9600);  //  setup serial
  // Display instructions
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("mode");
}

void loop() {
  timer.tick();
  timer_mode8_1.tick();
  timer_mode8_2.tick();
  readMode();
  handleButtonPress();
}

void readMode() {
  int val = analogRead(analogPin);  // read the input pin

  valSmooth = (valSmooth * (7) + val * 1) / 8;
  mode = 8 - (valSmooth * 9 / 1024);
  if (mode != prevMode) {

    lcd.clear();
    lcd.setCursor(0, 0);
    if (mode < 8) {
      lcd.print(modeLabel[mode]);
      lcd.setCursor(5, 0);
      lcd.print(mode);
    } else if (mode == 8) {
      lcd.print(modeState[5]);
      lcd.setCursor(5, 0);
      lcd.print(modeState[6]);
      lcd.setCursor(10, 0);
      lcd.print(modeState[7]);
    }

    Serial.print(modeLabel[mode]);
    Serial.println(mode);

    //adjusting based on distance
    if (mode == 0) {
      prevDistance = modeState[mode];
    }
    if ((mode != 0) && (prevMode == 0)) {
      int newDistance = modeState[0];
      if (newDistance != prevDistance) {
        for (int i = 2; i < 6; i++) {
          modeState[i] = modeState[i] * newDistance * newDistance / prevDistance / prevDistance;
        }
      }
    }
    if (mode == 0) {
      lcd.setCursor(0, 1);
      lcd.print("Prev ");

      lcd.setCursor(8, 1);
      lcd.print(prevDistance);
      lcd.print(" cm");
    }
    //adjusting for fstop

    if (mode == 1) {
      prevFstop = modeState[mode];
      lcd.setCursor(0, 1);
      lcd.print("Prev ");
      lcd.setCursor(8, 1);
      lcd.print(fstopList[prevFstop]);
    }
    if ((mode != 1) && (prevMode == 1)) {
      int deltaFstop = modeState[1] - prevFstop;
      if (deltaFstop != 0) {
        for (int i = 2; i < 6; i++) {
          modeState[i] *= pow(2, deltaFstop);
        }
        lcd.setCursor(0, 1);
        lcd.print("Timings adjusted");
      }
    }

    prevMode = mode;
    updateModeState();
  }
}


void updateModeState() {
  unsigned long value = modeState[mode];
  if (mode > 1) {
    lcd.setCursor(8, 0);
    lcd.print(value / 1000);
    lcd.print(".");
    lcd.print(value / 100 % 10);
    lcd.print("     ");
  } else if (mode == 0) {
    lcd.setCursor(8, 0);
    lcd.print(value);
    lcd.print(" cm     ");
  } else if (mode == 1) {
    lcd.setCursor(8, 0);
    lcd.print(fstopList[value]);
    lcd.print("      ");
  }
}

void exposureStart() {
  if ((mode > 0) && (mode < 6)) {
    digitalWrite(relay1pin, LOW);
    lcd.setCursor(0, 1);
    lcd.print("       ");
  }
  exposureRunning = true;

  click = false;
  exposureStartTime = millis();
}

void exposureStop() {
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  exposureRunning = false;
  timer.cancel();
  lcd.setCursor(0, 1);
  lcd.print("Stoped! ");
}

void exposureEnd() {
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  exposureRunning = false;
  timer.cancel();
  lcd.setCursor(0, 1);
  lcd.print("Done!  ");
  lcd.print("      ");
  click = false;
}

void exposureRemaining() {
  lcd.setCursor(8, 1);
  long exposureRemainingTime = (millis() - exposureStartTime);
  lcd.print(exposureRemainingTime / 1000);
  lcd.print(".");
  lcd.print(exposureRemainingTime / 100 % 10);
  lcd.print("      ");

  if ((exposureTimeTotal - exposureRemainingTime <= 5000) && (click == false) && ((mode > 5) && (mode < 8))) {
    click = true;
    digitalWrite(relay2pin, LOW);
  }
}

void lcdTest() {
  lcd.setCursor(0, 1);
  lcd.print("Test");
}

void handleButtonPress() {
  // Read button states
  bool secondsUp = !digitalRead(buttonSecondsUp);
  bool secondsDown = !digitalRead(buttonSecondsDown);
  bool switchDown = !digitalRead(pinSwitchDown);
  bool switchUp = !digitalRead(pinSwitchUp);
  bool start = !digitalRead(startButton);


  if (switchDown == true) {
    switchState = 2;
    if (mode == 0) {
      switchState = 0;
    }
  }
  if (switchUp == true) {
    switchState = 4;
    if (mode == 0) {
      switchState = 2;
    }
  }
  if ((switchDown == false) && (switchUp == false)) {
    switchState = 3;
    if (mode == 0) {
      switchState = 1;
    }
  }



  if (switchState != prevSwitchState) {
    stepScale = stepScaleList[switchState];

    prevSwitchState = switchState;
  }


  // changing the values
  if (mode == 0) {
    if ((secondsUp == true) && (secondsUp != prevSecondsUp) && ((millis() - lastSecondsUp) > debounceTime)) {
      if (modeState[mode] < 100000000) {
        modeState[mode] = modeState[mode] + stepScale;
      }
      updateModeState();
      lastSecondsUp = millis();
    }
    if ((secondsDown == true) && (secondsDown != prevSecondsDown) && ((millis() - lastSecondsDown) > debounceTime)) {
      if (modeState[mode] > stepScale) {
        modeState[mode] = modeState[mode] - stepScale;
      }
      updateModeState();
      lastSecondsDown = millis();
    }
  } else if (mode == 1) {
    if ((secondsUp == true) && (secondsUp != prevSecondsUp) && ((millis() - lastSecondsUp) > debounceTime)) {
      if (modeState[mode] < 10) {
        modeState[mode] = modeState[mode] + 1;
      }
      updateModeState();
      lastSecondsUp = millis();
    }
    if ((secondsDown == true) && (secondsDown != prevSecondsDown) && ((millis() - lastSecondsDown) > debounceTime)) {
      if (modeState[mode] > 0) {
        modeState[mode] = modeState[mode] - 1;
      }
      updateModeState();
      lastSecondsDown = millis();
    }
  } else if ((mode > 1) && (mode < 8)) {
    if ((secondsUp == true) && (secondsUp != prevSecondsUp) && ((millis() - lastSecondsUp) > debounceTime)) {
      if (modeState[mode] < 100000000) {
        modeState[mode] = modeState[mode] + stepScale;
      }
      updateModeState();
      lastSecondsUp = millis();
    }
    if ((secondsDown == true) && (secondsDown != prevSecondsDown) && ((millis() - lastSecondsDown) > debounceTime)) {
      if (modeState[mode] > stepScale) {
        modeState[mode] = modeState[mode] - stepScale;
      }
      updateModeState();
      lastSecondsDown = millis();
    }
      } else if (mode == 8) {
      //mode for running multiple timers
  
      if ((start == true) && (start != prevStart) && (modeState[5] > 0) && ((millis() - lastStart) > debounceTime)) {
        if (exposureRunning == false) {
          exposureStart();
          // Pass function pointer, do not call function
          timer.in(modeState[5], [](void*) -> bool { exposureStop(); return false; });
          timer.every(200, [](void*) -> bool { exposureRemaining(); return true; });
          exposureTimeTotal = modeState[5];
          lastStart = millis();
        } else {
          timer.cancel(); // Cancel all tasks on this timer instance
          exposureStop();
          lastStart = millis();
        }
      }
      if ((secondsDown == true) && (secondsDown != prevSecondsDown) && (modeState[6] > 0) && ((millis() - lastSecondsDown) > debounceTime)) {
        if (timer1Running == false) {
          timer_mode8_1.in(modeState[6], [](void*) -> bool {
             exposureEnd();
             timer1Running = false;
             return false;
          });
          timer1Running = true;
        } else {
          timer_mode8_1.cancel();
          timer1Running = false;
        }
      }
      if ((secondsUp == true) && (secondsUp != prevSecondsUp) && (modeState[7] > 0) && ((millis() - lastSecondsUp) > debounceTime)) {
        if (timer2Running == false) {
          timer_mode8_2.in(modeState[7], [](void*) -> bool {
             exposureEnd();
             timer2Running = false;
             return false;
          });
          timer2Running = true;
        } else {
          timer_mode8_2.cancel();
          timer2Running = false;
        }
      }
    }  //starting the timer
  if (mode != 8) {
    if ((start == true) && (start != prevStart) && (modeState[mode] > 0) && ((millis() - lastStart) > debounceTime)) {
      if (exposureRunning == false) {
        exposureStart();
        timer.in(modeState[mode], [](void*) -> bool { exposureEnd(); return false; });
        timer.every(200, [](void*) -> bool { exposureRemaining(); return true; });
        exposureTimeTotal = modeState[mode];
        lastStart = millis();
      } else {
        timer.cancel();
        exposureStop();
        lastStart = millis();
      }
    }
  }

  //recording button states

  prevSecondsUp = secondsUp;
  prevSecondsDown = secondsDown;
  prevStart = start;
}
