#include <arduino-timer.h>

#include <LiquidCrystal_I2C.h>

#include <Wire.h>

auto timer = timer_create_default();

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

unsigned long exposureTimeTotal;

byte switchState = 0, prevSwitchState = 0;

unsigned long modeState[9] = { 20, 0, 0, 0, 0, 0, 0, 0, 0 };
String modeLabel[9] = { "dist", "expo", "expo", "expo", "expo", "expo", "time", "time", "time" };
long exposureStartTime = 0;

int stepScale = 1000;
int stepScaleList[5] = { 1, 10, 100, 1000, 10000 };

int prevDistance = 20;

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
  readMode();
  handleButtonPress();
}

void readMode() {
  int val = analogRead(analogPin);  // read the input pin

  valSmooth = (valSmooth * (7) + val * 1) / 8;
  mode = 8 - (valSmooth * 9 / 1024);
  // mode = 8 - val * 9 / 1024;
  if (mode != prevMode) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(modeLabel[mode]);
    lcd.setCursor(5, 0);
    lcd.print(mode);

    Serial.print(modeLabel[mode]);
    Serial.println(mode);

    //adjusting based on distance
    if (mode == 0) {
      prevDistance = modeState[mode];
    }
    if ((mode != 0) && (prevMode == 0)) {
      //      float factor = modeState[0] * modeState[0] / prevDistance / prevDistance;
      int newDistance = modeState[0];
      for (int i = 1; i < 7; i++) {
        modeState[i] = modeState[i] * newDistance * newDistance / prevDistance / prevDistance;
      }
      lcd.setCursor(0, 1);
      lcd.print("Timings adjusted");
    }

    prevMode = mode;
    updateModeState();
  }

  if (mode == 0) {

    lcd.setCursor(0, 1);
    lcd.print("Prev ");

    lcd.setCursor(8, 1);
    lcd.print(prevDistance);
    lcd.print(" cm");
  }
}


void updateModeState() {
  unsigned long value = modeState[mode];
  if (mode > 0) {
    lcd.setCursor(8, 0);
    lcd.print(value / 1000);
    lcd.print(".");
    lcd.print(value / 100 % 10);
    lcd.print("     ");
  } else if (mode == 0) {
    lcd.setCursor(8, 0);
    lcd.print(value);
    lcd.print(" cm     ");
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

  // Serial.println("start");
  // lcd.setCursor(0, 1);
  // lcd.print("Start!        ");
}

void exposureStop() {
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  exposureRunning = false;
  timer.cancel();
  lcd.setCursor(0, 1);
  lcd.print("Stoped! ");
  // Serial.println("stop");
}

void exposureEnd() {
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  exposureRunning = false;
  timer.cancel();
  // int timeEllapsed = (millis() - exposureStartTime);
  // Serial.println("stop");
  lcd.setCursor(0, 1);
  lcd.print("Done!  ");
  // lcd.print(timeEllapsed);
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

  if ((exposureTimeTotal - exposureRemainingTime <= 5000) && (click == false) && ((mode > 5) && (mode < 9))) {
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
  // int val = analogRead(analogPin);  // read the input pin

  // valSmooth = (valSmooth * (3) + val * 1) / 4;
  // mode = 8 - (valSmooth * 9 / 1024);
  // if (mode != prevMode) {
  //   // lcd.clear();
  //   lcd.setCursor(0, 0);
  //   lcd.print("mode ");
  //   lcd.print(mode);
  //   updateModeState();
  //   prevMode = mode;
  // }

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

    // redundant debug code
    // lcd.setCursor(14, 1);
    // lcd.print(switchState);

    prevSwitchState = switchState;
  }


  // changing the values

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

  //starting the timer

  if ((start == true) && (start != prevStart) && (modeState[mode] > 0) && ((millis() - lastStart) > debounceTime)) {
    if (exposureRunning == false) {
      exposureStart();
      timer.in(modeState[mode], exposureEnd);
      timer.every(200, exposureRemaining);
      exposureTimeTotal = modeState[mode];
      lastStart = millis();
    } else {
      timer.cancel();
      exposureStop();
      lastStart = millis();
    }
  }

  //recordnign button states

  prevSecondsUp = secondsUp;
  prevSecondsDown = secondsDown;

  prevStart = start;
}
