#include <LiquidCrystal_I2C.h>

#include <Wire.h>

// Initialize the LCD, I2C address 0x3F, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Button pins
const int buttonSecondsUp = 2;
const int buttonSecondsDown = 3;
const int buttonTenthsUp = 4;
const int buttonTenthsDown = 5;
const int startButton = 6;

// Output pins

const int relay1pin = 7;
const int relay2pin = 8;


// Timer variables
int seconds = 0;
int tenths = 0;
int secondsCurrent = 0;
int tenthsCurrent = 0;
int tMax = 0;
int tCur = 0;
bool running = false;

// Previous button states for debouncing
bool prevSecondsUp = false, prevSecondsDown = false, prevTenthsUp = false, prevTenthsDown = false, prevStart = false;

void setup() {
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight

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

  // Display instructions
  lcd.setCursor(0, 0);
  lcd.print("Set Timer: ");
}

void loop() {
  // Check buttons and update timer values
  handleButtonPress();




  // Start the countdown if the timer is running
  if (running) {

    countdown();
  } else {

  }

}

void updateTCur() {

  lcd.setCursor(0, 1);
  lcd.print("tCur: ");
  lcd.print(secondsCurrent);
  lcd.print(".");
  lcd.print(tenthsCurrent);
  lcd.print("s  ");

}
void updateTMax() {
  lcd.setCursor(0, 0);
  lcd.print("tMax: ");
  lcd.print(seconds);
  lcd.print(".");
  lcd.print(tenths);
  lcd.print("s  ");
}

void handleButtonPress() {
  // Read button states
  bool secondsUp = !digitalRead(buttonSecondsUp);
  bool secondsDown = !digitalRead(buttonSecondsDown);
  bool tenthsUp = !digitalRead(buttonTenthsUp);
  bool tenthsDown = !digitalRead(buttonTenthsDown);
  bool start = !digitalRead(startButton);

  // Adjust seconds
  if (secondsUp && !prevSecondsUp) {
    seconds = (seconds + 1) % 100; // Max 99 seconds
    updateTMax();
  }
  if (secondsDown && !prevSecondsDown) {
    seconds = (seconds > 0) ? seconds - 1 : 0;

    updateTMax();
  }

  // Adjust tenths
  if (tenthsUp && !prevTenthsUp) {
    tenths = (tenths + 1) % 10; // Max 9 tenths

    updateTMax();
  }
  if (tenthsDown && !prevTenthsDown) {
    tenths = (tenths > 0) ? tenths - 1 : 0;

    updateTMax();
  }

  // Start the timer
  if (start && !prevStart && (seconds > 0 || tenths > 0)) {

    //turn on the relays
    digitalWrite(relay1pin, LOW);
    digitalWrite(relay2pin, LOW);

    tMax = seconds * 10 + tenths;
    tCur = 0;

    //reset the current time
    secondsCurrent = 0;
    tenthsCurrent = 0;

    //start the timer
    if (running == false) {
      running = true;
    } else {
      running = false;

      secondsCurrent = 0;
      tenthsCurrent = 0;
      tCur = 0;
      digitalWrite(relay1pin, HIGH);
      digitalWrite(relay2pin, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Timer Stopped");

    }
  }

  // Update previous button states
  prevSecondsUp = secondsUp;
  prevSecondsDown = secondsDown;
  prevTenthsUp = tenthsUp;
  prevTenthsDown = tenthsDown;
  prevStart = start;
}

void countdown() {
  delay(80); // Wait for 0.1 seconds (1/10 of a second)

  // Increase tenths and adjust seconds if needed
  if (tCur < tMax) {

    tCur++;

    if (tenthsCurrent < 9) {
      tenthsCurrent++;

    } else {
      secondsCurrent++;
      tenthsCurrent = 0;
    }

    updateTCur();

  } else {
    // Timer ends
    //turn off the relays
    digitalWrite(relay1pin, HIGH);
    digitalWrite(relay2pin, HIGH);

    running = false;

    //reset the current time
    secondsCurrent = 0;
    tenthsCurrent = 0;
    lcd.setCursor(0, 1);
    lcd.print("Time's Up!     ");
  }
}

