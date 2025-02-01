 #include <arduino-timer.h>

#include <LiquidCrystal_I2C.h>

#include <Wire.h>

auto timer = timer_create_default();

// Initialize the LCD, I2C address 0x3F, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Button pins
const int buttonSecondsUp = 2;
const int buttonSecondsDown = 3;
const int buttonTenthsUp = 4;
const int buttonTenthsDown = 5;
const int startButton = 6;
const int analogPin = A3;

// Output pins

const int relay1pin = 7;
const int relay2pin = 8;

// Mode vals

byte mode = 0;
byte prevMode = 0;
int valSmooth = 0;

bool exposureRunning = false;

int modeState[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int exposureStartTime = 0;

// Previous button states for debouncing
bool prevSecondsUp = false, prevSecondsDown = false, prevTenthsUp = false, prevTenthsDown = false, prevStart = false;


void setup() {
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight

  // Configure button pins
  pinMode(buttonSecondsUp, INPUT_PULLUP);
  pinMode(buttonSecondsDown, INPUT_PULLUP);
  pinMode(buttonTenthsUp, INPUT_PULLUP);
  pinMode(buttonTenthsDown, INPUT_PULLUP);
  pinMode(startButton, INPUT_PULLUP);
  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);

  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);

  Serial.begin(9600);  //  setup serial
  // Display instructions
  lcd.setCursor(0, 0);
}

void loop() {
  timer.tick();
  handleButtonPress();
}

void updateModeState() {
  lcd.setCursor(8, 0);
  lcd.print(modeState[mode] / 1000);
  lcd.print(",");
  lcd.print(modeState[mode] / 100 % 10);
  lcd.print("     ");
}

void exposureStart() {
  exposureRunning = true;
  digitalWrite(relay1pin, LOW);
  digitalWrite(relay2pin, LOW);
  lcd.setCursor(0, 1);
  lcd.print("Start!        ");
  exposureStartTime = millis();
}

void exposureStop() {
  exposureRunning = false;
  timer.cancel();
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("Stoped! ");
}

void exposureEnd() {
  exposureRunning = false;
  timer.cancel();
  digitalWrite(relay1pin, HIGH);
  digitalWrite(relay2pin, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("Done!  ");
  lcd.print(millis() - exposureStartTime);
  lcd.print("      ");
}

void exposureRemaining() {
  lcd.setCursor(8, 1);
  int exposureRemainingTime = (millis() - exposureStartTime);
  lcd.print(exposureRemainingTime / 1000);
    lcd.print(",");
  lcd.print(exposureRemainingTime / 100 % 10);
  lcd.print("      ");
}

void lcdTest() {
    lcd.setCursor(0, 1);
      lcd.print("Test");
}

void handleButtonPress() {
  // Read button states
  bool secondsUp = !digitalRead(buttonSecondsUp);
  bool secondsDown = !digitalRead(buttonSecondsDown);
  bool tenthsUp = !digitalRead(buttonTenthsUp);
  bool tenthsDown = !digitalRead(buttonTenthsDown);
  bool start = !digitalRead(startButton);

  int val = analogRead(analogPin);  // read the input pin

  valSmooth = (valSmooth * (3) + val * 1) / 4;
  mode = valSmooth * 9 / 1024;

  if (mode != prevMode) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("mode ");
    lcd.print(mode);
    updateModeState();
    prevMode = mode;
  }

  if ((secondsUp == true) && (secondsUp != prevSecondsUp)) {
    if (modeState[mode] < 1000000) {
      modeState[mode] = modeState[mode] + 1000;
    }
    updateModeState();
  }

  if ((secondsDown == true) && (secondsDown != prevSecondsDown)) {
    if (modeState[mode] > 1000) {
      modeState[mode] = modeState[mode] - 1000;
    }
    updateModeState();
  }

  if ((tenthsUp == true) && (tenthsUp != prevTenthsUp)) {
    if (modeState[mode] < 1000000) {
      modeState[mode] = modeState[mode] + 100;
    }
    updateModeState();
  }

  if ((tenthsDown == true) && (tenthsDown != prevTenthsDown)) {
    if (modeState[mode] > 100) {
      modeState[mode] = modeState[mode] - 100;
    }
    updateModeState();
  }

  if ((start == true) && (start != prevStart) && (modeState[mode] > 0)) {
    if (exposureRunning == false) {
      exposureStart();
      timer.in(modeState[mode], exposureEnd);
      timer.every(100, exposureRemaining);
    } else {
      timer.cancel();
      exposureStop();
    }
  }

  prevSecondsUp = secondsUp;
  prevSecondsDown = secondsDown;
  prevTenthsUp = tenthsUp;
  prevTenthsDown = tenthsDown;
  prevStart = start;
}
